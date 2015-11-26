/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
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

	/* this object is on our heap */
	/* free(obj->obj); */
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

//void gc_scavenge() {
//	segment_t *swap_ptr;
//
//	/* swap from-space to to-space */
//	swap_ptr = segments_from_space_g;
//	segments_from_space_g = segments_to_space_g;
//	segments_to_space_g = swap_ptr;

/* TODO */

/*
 def scavenge():
 swap(fromSpace, toSpace)
 allocationPtr = toSpace.bottom
 scanPtr = toSpace.bottom

 for i = 0..len(roots):
 root = roots[i]
 if inFromSpace(root):
 rootCopy = copyObject(&allocationPtr, root)
 setForwardingAddress(root, rootCopy)
 roots[i] = rootCopy

 while scanPtr < allocationPtr:
 obj = object at scanPtr
 scanPtr += size(obj)
 n = sizeInWords(obj)
 for i = 0..n:
 if isPointer(obj[i]) and not inOldSpace(obj[i]):
 fromNeighbor = obj[i]
 if hasForwardingAddress(fromNeighbor):
 toNeighbor = getForwardingAddress(fromNeighbor)
 else:
 toNeighbor = copyObject(&allocationPtr, fromNeighbor)
 setForwardingAddress(fromNeighbor, toNeighbor)
 obj[i] = toNeighbor

 def copyObject(*allocationPtr, object):
 copy = *allocationPtr
 *allocationPtr += size(object)
 memcpy(copy, object, size(object))
 return copy
 */
//}
/******************************************************************************/
/* memory managment */

segment_t *segments_g = NULL;

segment_t *mem_segment_init(size_t size) {
	segment_t *s;

	vm_debug(DBG_MEM, "mem_segment_init(size=%u)\n", size);

	assert(size > 0);

	s = malloc(sizeof(segment_t) + (size - 1) * OBJ_ALIGN);
	assert(s);

	vm_debug(DBG_MEM, "segment ptr=%p\n", s);

	s->size = size;
	s->used = 0;
	s->next = NULL;
	s->beginning = s->data;
	s->end = s->beginning;

	/* in the segment_t, there is 1 data. but the s->beginning points to the
	 * 0th data.
	 */
	assert((uint8_t *) s + sizeof(segment_t) + (size - 1) * OBJ_ALIGN == //
			(uint8_t *) s->beginning + size * OBJ_ALIGN);

	/* FIXME */
	vm_debug(DBG_MEM,
			"begin=%p\nsegment+header+data*size=\t%p\ndataptr+size=\t%p\n",
			s->beginning, //
			(uint8_t *) s + sizeof(segment_t) + (size - 1) * OBJ_ALIGN, //
			(uint8_t *) s->beginning + size * OBJ_ALIGN);

	if (segments_g == NULL) {
		segments_g = s;
	}// else {
//		assert(0 && "haha, no realloc yet");
//		segments_g->next = s;
//		segments_g = s;
//		}

	return (s);
}

bool mem_init() {
	segments_g = mem_segment_init(SEGMENT_SIZE);
	return (true);
}

bool mem_free() {
	segment_t *ptr;
	segment_t *next;

	next = segments_g;
	while (next != NULL) {
		ptr = next;
		next = next->next;
		free(ptr);
	}

	return (true);
}

void *mem_segment_malloc(size_t size) {
	void *ptr;
	segment_t *new;

	size = ALIGNED(size);

	assert(segments_g != NULL);

	vm_debug(DBG_MEM, "sizeof double = %u\n", sizeof(double));
	vm_debug(DBG_MEM, "sizeof size_t = %u\n", sizeof(size_t));
	vm_debug(DBG_MEM, "sizeof uint8_t = %u\n", sizeof(uint8_t));
	vm_debug(DBG_MEM, "sizeof segment_t = %u\n", sizeof(segment_t));
	vm_debug(DBG_MEM, "sizeof segment_t* = %u\n", sizeof(segment_t *));
	vm_debug(DBG_MEM, "sizeof void* = %u\n", sizeof(void *));

	vm_debug(DBG_MEM, "mem_segment_malloc(size=%u, %#08x)\n", size, size);
	vm_debug(DBG_MEM, "segment=\t%p (%lu)\n", segments_g, segments_g);
	vm_debug(DBG_MEM, "totalend=\t%p (%lu)\n",
			(data_t *) segments_g->beginning + segments_g->size,
			(data_t *) segments_g->beginning + segments_g->size);
	vm_debug(DBG_MEM, "beginnning=\t%p (%lu)\n", segments_g->beginning,
			segments_g->beginning);
	vm_debug(DBG_MEM, "before: end=\t%p (%lu)\n", segments_g->end,
			segments_g->end);

	if (segments_g->used + size > segments_g->size) {
		vm_debug(DBG_MEM, "We need to allocate a new segment.\n");
		/*
			old:
			[current]->[old1]->[old2]
			segments_g points to current

		 	new:
		 	[new]->[current]->[old1]->[old2]
		 	new->next points to current
		 	segments_g points to new
		 */
		new = mem_segment_init(SEGMENT_SIZE);
		new->next = segments_g;
		segments_g = new;
	}

	ptr = segments_g->end;
	segments_g->used += size;
	segments_g->end =
			(data_t *) ((uint8_t *) segments_g->end + size * OBJ_ALIGN);

	vm_debug(DBG_MEM, "after: end=\t%p (%lu)\n", segments_g->end,
			segments_g->end);

	return (ptr);
}

void *mem_segment_calloc(size_t size) {
	void *ptr;

	ptr = mem_segment_malloc(size);
	memset(ptr, 0, size);

	return (ptr);
}

/* (c)allocate memory for an object, set its header and return a pointer */
inline void *mem_obj_malloc(type_t type, class_t *cls, size_t size) {
	kek_obj_t *obj;

	obj = mem_segment_malloc(size);
	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;

	gc_obj_add(obj, size);

	return (obj);
}

inline void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size) {
	kek_obj_t *obj;

	obj = mem_segment_calloc(num * size);
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
	vm_debug(DBG_GC, "gc %d\n", sysconf(_SC_PAGESIZE));
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
