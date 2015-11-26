/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdbool.h>

#include "vm.h"

#define ARR_INIT_SIZE 512

struct _class;

// Integers are primitive objects without methods - they don't need a class pointer
union _kek_obj * alloc_integer(void);
union _kek_obj * alloc_array(struct _class * arr_class);
void alloc_arr_elems(struct _kek_array * arr);
union _kek_obj ** alloc_const_arr_elems(int length);
void realloc_arr_elems(struct _kek_array * arr, int length);
union _kek_obj * alloc_string(struct _class * str_class, int length);
union _kek_obj * alloc_exception(struct _class * expt_class);
union _kek_obj * alloc_udo(struct _class * arr_class);
union _kek_obj * alloc_file(struct _class * file_class);

/******************************************************************************/
/* memory managment */

/* http://jayconrod.com/posts/55/a-tour-of-v8-garbage-collection */
/* http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.63.6386&rep=rep1&type=pdf */

/* from claus */
#define SEGMENT_SIZE (2*1024) /* FIXME TODO 2KB for now */
typedef double data_t; /* data type */
#define OBJ_ALIGN sizeof(double)
#define ALIGNED(n) (((n) + OBJ_ALIGN-1) & ~(OBJ_ALIGN-1))
#define ALIGNED_SIZE_OF(obj) ALIGNED((obj)->h.size)

/* this will main call to initialiaze everything and then free */
bool mem_init(void);
bool mem_free(void);

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



/*

|header|data|

 */
typedef struct _segment {
	size_t size;
	size_t used;
	//segment_slots_buffer_t *slots_buffer;

	void *beginning; /* pointer to the start of the data */
	void *end; /* pointer to the end of the data of this segment */

	struct _segment *next;

	double data[1];
	/* data[size-1] */
} segment_t;

extern segment_t *segments_from_space_g;
extern segment_t *segments_to_space_g;
extern segment_t *segments_old_space_g;

/******************************************************************************/
/* gc */

typedef struct _gc_obj {
	kek_obj_t *obj;
	size_t size;
	struct _gc_obj *next;
} gc_obj_t;

#define GC_TICKS_DEFAULT 10
extern int gc_ticks_g; /* how often will gc run */
extern gc_obj_t *gc_obj_g;
extern gc_obj_t *gc_obj_root_g;

/* this function will be called from the main loop in vm */
void gc(void);
void gc_delete_all(void);

/* gc for young objects. objs who will survive 2 young gc r old */
/* https://en.wikipedia.org/wiki/Cheney's_algorithm */
void gc_scavenge(void);



/******************************************************************************/

#endif
