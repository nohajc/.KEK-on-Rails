/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

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
union _kek_obj * alloc_udo(struct _class * arr_class);
union _kek_obj * alloc_file(struct _class * file_class);



/******************************************************************************/
/* memory managment */

/* from claus */
#define SEGMENT_SIZE (64*1024)
#define OBJ_ALIGN sizeof(double)
#define ALIGNED(n) (((n) + OBJ_ALIGN-1) & ~(OBJ_ALIGN-1))
#define ALIGNED_SIZE_OF(obj) ALIGNED((obj)->h.size)

typedef struct _segment {
	size_t size;
	struct _segment *next;
} segment_t;

extern segment_t *segments_g;

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


/******************************************************************************/

#endif
