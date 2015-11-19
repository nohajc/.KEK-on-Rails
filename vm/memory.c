/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <assert.h>
#include "memory.h"

/******************************************************************************/
/* memory managment */

segment_t *segments_g = NULL;

segment_t *mem_segment_init(size_t size) {
	segment_t *s;

	s = malloc(sizeof(segment_t) + size);
	assert(s);

	s->size = size;
	s->next = NULL;

	if (segments_g == NULL) {
		segments_g = s;
	} else {
		segments_g->next = s;
		segments_g = s;
	}

	return (s);
}

/* (c)allocate memory for an object, set its header and return a pointer */
inline void *mem_obj_malloc(type_t type, class_t *cls, size_t size) {
	kek_obj_t *obj;

	obj = malloc(size);
	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;
	obj->h.size = size;

	return (obj);
}

inline void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size) {
	kek_obj_t *obj;

	obj = calloc(num, size);
	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;
	obj->h.size = size;

	return (obj);
}

/******************************************************************************/
/* gc */

int gc_ticks_g = GC_TICKS_DEFAULT;

void gc() {
	vm_debug(DBG_GC, "gc\n");
}

/******************************************************************************/

kek_obj_t * alloc_array(class_t * arr_class) {
	return (mem_obj_malloc(KEK_ARR, arr_class, sizeof(kek_array_t)));
}

void alloc_arr_elems(kek_array_t * arr) {
	/* TODO: mem_malloc */
	arr->elems = malloc(ARR_INIT_SIZE * sizeof(kek_obj_t*));
	assert(arr->elems);
	arr->alloc_size = ARR_INIT_SIZE;
}

void realloc_arr_elems(struct _kek_array * arr, int length) {
	while (arr->alloc_size < length) {
		arr->alloc_size *= 2;
	}
	/* TODO: mem_realloc? */
	arr->elems = realloc(arr->elems, arr->alloc_size * sizeof(kek_obj_t*));
	assert(arr->elems);
}

kek_obj_t * alloc_string(class_t * str_class, int length) {
	return (mem_obj_malloc(KEK_STR, str_class, sizeof(kek_string_t) + length));
}

kek_obj_t * alloc_integer(void) {
	return (mem_obj_malloc(KEK_INT, NULL, sizeof(kek_int_t)));
}

kek_obj_t * alloc_udo(class_t * udo_class) {
	/* Calloc is used to avoid valgrind warnings about invalid reads/writes
	 * to uninitialized memory */
	return (mem_obj_calloc(KEK_UDO, udo_class, udo_class->syms_instance_cnt - 1,
			sizeof(kek_udo_t)));
}

kek_obj_t * alloc_file(class_t * file_class) {
	return (mem_obj_malloc(KEK_FILE, file_class, sizeof(kek_file_t)));
}
