/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>

#include "vm.h"

// For NEW_SEGMENT_SIZE 1024*10
// The array in gc_arrloop.kexe is too big.
// We need to detect it does not fit into segment
// and possibly allocate it in old space.
#define ARR_INIT_SIZE 512

struct _class;

// Integers are primitive objects without methods - they don't need a class
// pointer
union _kek_obj * alloc_integer(void);
union _kek_obj * alloc_array(struct _class * arr_class);
// size = allocated, length = number of items
union _kek_obj ** alloc_arr_elems(int size, int length);
union _kek_obj ** alloc_const_arr_elems(int length);
void arr_realloc_elems(struct _kek_array * arr, int length);
union _kek_obj * alloc_string(struct _class * str_class, int length);
union _kek_obj * alloc_symbol(int length);
union _kek_obj * alloc_exception(struct _class * expt_class);
union _kek_obj * alloc_udo(struct _class * arr_class);
union _kek_obj * alloc_file(struct _class * file_class);
union _kek_obj * alloc_term(struct _class * term_class);

/******************************************************************************/
/* memory managment */

/* jayconrod.com/posts/55/a-tour-of-v8-garbage-collection */
/* citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.63.6386&rep=rep1&type=pdf */

/* from claus */
#define SEGMENT_SIZE (4096*4096*2) /* FIXME TODO creating new segments is disabled
 just make sure that this is big enough when using -gx */
#define OBJ_ALIGN 8
#define ALIGNED(n) ((((n) + OBJ_ALIGN-1) & ~(OBJ_ALIGN-1)))

/* Remember set */
typedef struct _segment_slots_buffer {
	/* TODO */
	kek_obj_t *obj;
	struct _segment_slots_buffer *next;
} segment_slots_buffer_t;

/* pointers from old to new space */
typedef struct _segment_write_barrier {
	/* TODO */
	kek_obj_t *obj;
	struct _segment_slots_buffer *next;
} segment_write_barrier_t;

typedef struct _segment_new {
	int a;
} segment_new_t;

typedef struct _segment_old {
	int a;
} segment_old_t;

typedef union {
	segment_new_t *hdr_new;
	segment_old_t *hdr_old;
} segment_header_t;

typedef struct _segment {
	segment_header_t header;
	size_t size;
	size_t used;
	//segment_slots_buffer_t *slots_buffer;

	void *beginning; /* pointer to the start of the data */
	void *end; /* pointer to the end of the data of this segment */

	struct _segment *next;
} segment_t;

extern segment_t *segments_old_space_g;

/* this will main call to initialiaze everything and then free */
bool mem_init(void);
bool mem_free(void);
segment_t *mem_segment_init(size_t size);

void *mem_obj_malloc(type_t type, class_t *cls, size_t size);
void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size);

/******************************************************************************/
/* obj_table */

typedef struct _obj_table {
	obj_state_t state;
	kek_obj_t *obj_ptr;

	/* array of pointers that points to this obj */
	/* we'll asume that most to most objt will point only one ptr */
	kek_obj_t **ptr;

	uint32_t ptr_arr_cnt;
	uint32_t ptr_arr_size;
	kek_obj_t ***ptr_arr;
} obj_table_t;

extern obj_table_t *obj_table_g;
extern uint32_t obj_table_size_g;
#define REF(obj) (*(obj))
#define OBJ_TABLE_DEFAULT_SIZE 2048
#define OBJ_TABLE_PTR_ARR_DEFAULT_SIZE 256
void obj_table_init(void);
void obj_table_free(void);

/* argument is a pointer to the pointer to the object.
 * we need it for updating when the obj moves in the heap */
uint32_t obj_table_regptr(kek_obj_t **);

/******************************************************************************/
/* gc */

typedef enum _gc_type {
	GC_NONE, //
	GC_NEW, // just new space (cheney only)
	GC_GEN // generational GC. new and old space
} gc_type_t;

//#define JUST_USE_MALLOC

//#define GC_TYPE_DEFAULT GC_NEW
#define GC_TYPE_DEFAULT GC_GEN
extern gc_type_t gc_type_g;

typedef struct _gc_obj {
	kek_obj_t *obj;
	size_t size;
	struct _gc_obj *next;
} gc_obj_t;

typedef struct _gc_carrlist {
	kek_array_t *arr;
	struct _gc_carrlist *next;
} gc_carrlist_t;

typedef struct _gc_rootset {
	struct _gc_rootset *next;
	kek_obj_t **obj;
	uint32_t uid;
} gc_rootset_t;
extern gc_rootset_t *gc_rootset_g;
extern uint32_t gc_rootset_last_uid_g;
uint32_t gc_rootset_add(kek_obj_t **obj);
void gc_rootset_remove_id(uint32_t id);
//void gc_rootset_remove_ptr(kek_obj_t **obj);
void gc_rootset_init(void);
void gc_rootset_free(void);

#define GC_TICKS_DEFAULT 10000
extern int gc_ticks_g; /* how often will gc run */
extern gc_obj_t *gc_obj_g;
extern gc_obj_t *gc_obj_root_g;
extern gc_carrlist_t *gc_carrlist_root_g;

/* GC API */
void gc(void);
void gc_lock(void);
void gc_unlock(void);
double gc_remaining(void);
void *gc_obj_malloc(type_t type, class_t *cls, size_t size);
bool gc_in_new(void *ptr, size_t size);
bool gc_in_old(void *ptr, size_t size);

void gc_init(void);
void gc_free(void);
void gc_delete_all(void);
void gc_rootset(void (*fn)(kek_obj_t **));

/******************************************************************************/
/* cheney */

#define NEW_SEGMENT_SIZE (1024*512)
extern void *segments_from_space_g;
extern void *segments_to_space_g;
extern void *to_space_free_g; /* points to the end of data in from-space */
extern size_t to_space_size_g;
extern void *scan_ptr_g;
extern uint32_t gc_cheney_iteration_t;

bool gc_cheney_ptr_in_from_space(void *, size_t);
bool gc_cheney_ptr_in_to_space(void *, size_t);

void gc_cheney_init(void);
void gc_cheney_free(void);
bool gc_cheney_can_malloc(size_t size);
void *gc_cheney_malloc(type_t type, class_t *cls, size_t size);
void *gc_cheney_calloc(type_t type, class_t *cls, size_t size);
void gc_cheney_scavenge();

/******************************************************************************/
/* old space */

typedef struct _os_item {
	struct _os_item *next;
	kek_obj_t *obj;
} os_item_t;

typedef struct _os_remember_set {
	struct _os_remember_set *next;
	kek_obj_t *old_obj;
	kek_obj_t **new_obj;
} os_remember_set_t;

extern os_remember_set_t *gc_os_remember_set_g;
extern os_item_t *gc_os_items_g; /* linked list of items in old space */

void gc_os_init();

/* free all objects in the old space, its wrappers and remember set */
void gc_os_free();

void gc_os_rec_cpy_neighbors(kek_obj_t **);

/* move the object to the old space with all its neighbors */
kek_obj_t *gc_os_add_item(kek_obj_t **);

/* return true, if the object is in the old/new space */
bool gc_os_is_in_old(kek_obj_t *);
bool gc_os_is_in_new(kek_obj_t *);

/* this is called in executing ST bytecode */
void gc_os_write_barrier(kek_obj_t *dst_obj, kek_obj_t **dst_addr);

int gc_os_items_cnt(void);
int gc_os_rs_items_cnt(void);

/******************************************************************************/

#endif
