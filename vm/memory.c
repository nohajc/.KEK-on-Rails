/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <assert.h>
#include "memory.h"

gc_obj_t *gc_obj_g = NULL;
gc_obj_t *gc_obj_root_g = NULL;

void gc_obj_add(kek_obj_t *obj, size_t size) {
	gc_obj_t *go;

	vm_debug(DBG_GC, "gc_obj_add objptr=%p\n");

	go = malloc(sizeof(gc_obj_t));
	assert(go);

	go->obj = obj;
	go->size = size;
	go->next = NULL;

	if (gc_obj_root_g == NULL) {
		gc_obj_root_g = go;
		gc_obj_g = gc_obj_root_g;
	} else {
		gc_obj_g->next = go;
		gc_obj_g = go;
	}
}

void gc_obj_free(gc_obj_t *obj) {
	switch (obj->obj->h.t) {
	case KEK_ARR:
		/* TODO FIXME: shouldn't we delete also the elems? */
		free(obj->obj->k_arr.elems);
		break;
	default:
		break;
	}

	free(obj->obj);
	free(obj);
}

void gc_delete_all() {
	gc_obj_t *gcptr = gc_obj_root_g;
	gc_obj_t *gcptr_next;

	vm_debug(DBG_GC, "gc_delete_all\n");

	while (gcptr != NULL) {
		vm_debug(DBG_GC, "gc_delete_all objptr=%p\n", gcptr->obj);
		gcptr_next = gcptr->next;
		gc_obj_free(gcptr);
		gcptr = gcptr_next;
	}
}

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

	gc_obj_add(obj, size);

	return (obj);
}

inline void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size) {
	kek_obj_t *obj;

	obj = calloc(num, size);
	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;

	gc_obj_add(obj, size);

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

kek_obj_t ** alloc_const_arr_elems(int length) {
	kek_obj_t ** elems = malloc(length * sizeof(kek_obj_t*));
	assert(elems);
	return elems;
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
	/* Calloc is used to avoid valgrind warnings about
	 * invalid reads/writes to uninitialized memory */
	uint32_t syms_cnt = udo_class->syms_instance_cnt;
	if (syms_cnt) {
		// Add symbols from parents
		syms_cnt += udo_class->syms_instance[0].addr;
	}

	return (mem_obj_calloc(KEK_UDO, udo_class,
			sizeof(kek_udo_t) + (syms_cnt - 1) * sizeof(kek_obj_t), 1));
}

kek_obj_t * alloc_file(class_t * file_class) {
	return (mem_obj_malloc(KEK_FILE, file_class, sizeof(kek_file_t)));
}
