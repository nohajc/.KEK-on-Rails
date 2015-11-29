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

/******************************************************************************/
/* Cheney */
/* https://en.wikipedia.org/wiki/Cheney's_algorithm */
/* http://jayconrod.com/posts/55/a-tour-of-v8-garbage-collection */

/* global variables */
segment_t *segments_from_space_g = NULL;
segment_t *segments_to_space_g = NULL;
void *from_space_free_g;
void *alloc_ptr_g;
void *scan_ptr_g;

bool gc_in_from_space(kek_obj_t *obj) {
	return (obj->h.copied == false);
}

kek_obj_t *gc_cheney_copy_obj(kek_obj_t *obj) {
	kek_obj_t *ret;
	ret = memcpy(alloc_ptr_g, obj, sizeof(*obj));
	alloc_ptr_g = ((uint8_t *) alloc_ptr_g + sizeof(*obj));
	return (ret);
}

void gc_cheney_copy_roots(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *copy;

	assert(IS_PTR(obj));

	if (vm_is_const(obj)) {
		return;
	}

	if (gc_in_from_space(obj)) {
		copy = gc_cheney_copy_obj(obj);
		obj->h.forwarding_address = copy;
		obj = copy;
		obj->h.survived++;
		obj->h.copied = true;
	}
}

void gc_cheney_reset_copied(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;

	obj->h.copied = false;
}

void gc_cheney_scavenge() {
	segment_t *swap_ptr;
	kek_obj_t *obj;
	kek_obj_t *obj_inner;
	kek_obj_t *from_neighbor;
	kek_obj_t *to_neighbor;
	uint8_t i;

	vm_debug(DBG_MEM, "gc_cheney_scavenge()\n");

	/* swap from-space to to-space */
	swap_ptr = segments_from_space_g;
	segments_from_space_g = segments_to_space_g;
	segments_to_space_g = swap_ptr;

	/* set alloc and scan ptrs to the beginning of the to-space segment */
	alloc_ptr_g = segments_to_space_g;
	scan_ptr_g = segments_to_space_g;

	gc_rootset(gc_cheney_reset_copied);

	/* copy roots to to-space and actualize alloc ptr */
	gc_rootset(gc_cheney_copy_roots);

	while (scan_ptr_g < alloc_ptr_g) {
		obj = (kek_obj_t *) scan_ptr_g;
		assert(IS_PTR(obj));
		scan_ptr_g = ((uint8_t *) scan_ptr_g) + sizeof(*obj);

		for (i = 0; i < sizeof(*obj); i++) {
			obj_inner = (kek_obj_t *) (((uint8_t *) obj) + i);

			if (IS_PTR(obj_inner) /* TODO: not in old space */) {
				if (vm_is_const(obj_inner)) {
					continue;
				}

				from_neighbor = obj_inner;

				if (from_neighbor->h.forwarding_address != NULL) {
					to_neighbor = from_neighbor->h.forwarding_address;
				} else {
					to_neighbor = gc_cheney_copy_obj(from_neighbor);
					from_neighbor->h.forwarding_address = to_neighbor;
				}

				obj_inner = to_neighbor;
			}
		}
	}
}

void gc_cheney_init() {
	segments_from_space_g = mem_segment_init(SEGMENT_SIZE);
	segments_to_space_g = mem_segment_init(SEGMENT_SIZE);

	from_space_free_g = segments_from_space_g;
}

void gc_cheney_free() {
	free(segments_from_space_g);
	free(segments_to_space_g);
}

void *gc_cheney_malloc(type_t type, class_t *cls, size_t size) {
	void *ptr;
	segment_t *new;

	size = ALIGNED(size);

	assert(segments_from_space_g != NULL);

	if (((uint8_t *) from_space_free_g + size) > //
			(uint8_t *) segments_from_space_g + (size_t) NEW_SEGMENT_SIZE) {
		vm_debug(DBG_MEM, "gc_cheney_malloc: From space needs GC.\n");
		gc_cheney_scavenge();
	}

	ptr = from_space_free_g;
	from_space_free_g = ((uint8_t *) from_space_free_g) + size * OBJ_ALIGN;

	vm_debug(DBG_MEM, "gc_cheney_malloc: from=%p to=%p\n", ptr,
			from_space_free_g);

	((kek_obj_t *) ptr)->h.t = type;
	((kek_obj_t *) ptr)->h.cls = cls;
	((kek_obj_t *) ptr)->h.forwarding_address = false;
	((kek_obj_t *) ptr)->h.survived = 0;
	((kek_obj_t *) ptr)->h.copied = false;

	return (ptr);
}

void *gc_cheney_calloc(type_t type, class_t *cls, size_t size) {
	void *ptr;

	ptr = gc_cheney_malloc(type, cls, size);
	memset(ptr, 0, size);

	return (ptr);
}

/******************************************************************************/
/* memory managment */

segment_t *segments_g = NULL;

segment_t *mem_segment_init(size_t size) {
	segment_t *s;

	assert(size > 0);

	s = malloc(sizeof(segment_t) + (size - 1) * OBJ_ALIGN);
	assert(s);

	vm_debug(DBG_MEM, "mem_segment_init(size=%u), segment ptr=%p\n", size, s);

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

	return (s);
}

bool mem_init() {
	gc_init();
	return (true);
}

bool mem_free() {
	gc_free();
	return (true);
}

void *mem_segment_malloc(size_t size) {
	void *ptr;
	segment_t *new;

	size = ALIGNED(size);

	assert(segments_g != NULL);

	if (segments_g->used + size > segments_g->size) {
		vm_debug(DBG_MEM, "We need to allocate a new segment.\n");
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
/* obj_table */

obj_table_t *obj_table_g;
uint32_t obj_table_size_g;

void obj_table_init(void) {
	vm_debug(DBG_OBJ_TBL, "Init!");
	obj_table_size_g = OBJ_TABLE_DEFAULT_SIZE;
	obj_table_g = calloc(obj_table_size_g, sizeof(obj_table_t));
	assert(obj_table_g);
}

void obj_table_free(void) {
	vm_debug(DBG_OBJ_TBL, "Free!");
	free(obj_table_g);
}

void obj_table_print() {
	uint32_t i;
	vm_debug(DBG_OBJ_TBL, "-------------- obj_table_print() ------------\n");
	for (i = 0; i < obj_table_size_g; i++) {
		if (obj_table_g[i].obj_ptr != NULL) {
			vm_debug(DBG_OBJ_TBL, "%d: obj=%p from=%p from_arr=%p\n", i,
					obj_table_g[i].obj_ptr, obj_table_g[i].ptr,
					obj_table_g[i].ptr_arr);
		}
	}
}

static uint32_t obj_table_find(kek_obj_t *obj) {
	uint32_t i;

	for (i = 0; i < obj_table_size_g; i++) {
		if (obj_table_g[i].obj_ptr == obj) {
			vm_debug(DBG_OBJ_TBL, "obj_table_find(%p) found %d\n", obj, i);
			return (i);
		}
	}

	vm_debug(DBG_OBJ_TBL, "obj_table_find(%p) not found\n", obj);
	return (UINT32_MAX);
}

uint32_t obj_table_add(kek_obj_t **objptr, kek_obj_t *obj) {
	uint32_t i;

	for (i = 0; i < obj_table_size_g; i++) {
		if (obj_table_g[i].obj_ptr == NULL) {
			obj_table_g[i].obj_ptr = obj;
			obj_table_g[i].ptr = objptr;
			obj_table_g[i].state = OBJ_1ST_GEN_YOUNG;

			vm_debug(DBG_OBJ_TBL,
					"obj_table_add(whoami=%p, obj=%p) added on %d\n", objptr,
					obj, i);
			return (i);
		}
	}

	/* if there is no more place, realloc */
	vm_debug(DBG_OBJ_TBL, "obj_table_add(whoami=%p, obj=%p) realloced\n",
			objptr, obj);
	obj_table_size_g *= 2;
	obj_table_g = realloc(obj_table_g, obj_table_size_g * sizeof(obj_table_t));
	assert(obj_table_g);
	memset(&obj_table_g[obj_table_size_g / 2], 0, obj_table_size_g / 2);

	obj_table_g[obj_table_size_g / 2].obj_ptr = obj;
	obj_table_g[obj_table_size_g / 2].ptr = objptr;
	obj_table_g[obj_table_size_g / 2].state = OBJ_1ST_GEN_YOUNG;

	return (obj_table_size_g / 2);
}

uint32_t obj_table_regptr(kek_obj_t **objptr) {
	uint32_t i;
	kek_obj_t *obj;

	obj = *objptr;

	assert(objptr);
	assert(obj);

	i = obj_table_find(obj);
	vm_debug(DBG_OBJ_TBL, "obj_table_getptr(whoami=%p, obj=%p) i=%d\n", objptr,
			obj, i);
	if (i == UINT32_MAX) {
		/* obj is not in table yet */
		return (obj_table_add(objptr, obj));
	} else {
		/* obj is on index i */
		assert(obj_table_g[i].obj_ptr == obj);
		assert(obj_table_g[i].ptr != NULL);

		/* now add whoisit to the pointer array */
		if (obj_table_g[i].ptr_arr == NULL) {
			vm_debug(DBG_OBJ_TBL, "obj_table_g[%d]->ptr_arr malloc\n", i);
			obj_table_g[i].ptr_arr_size = OBJ_TABLE_PTR_ARR_DEFAULT_SIZE;
			obj_table_g[i].ptr_arr_cnt = 0;
			obj_table_g[i].ptr_arr = malloc(
					obj_table_g[i].ptr_arr_size * sizeof(kek_obj_t **));
			assert(obj_table_g[i].ptr_arr);
		}

		if (obj_table_g[i].ptr_arr_cnt == obj_table_g[i].ptr_arr_size) {
			vm_debug(DBG_OBJ_TBL, "obj_table_g[%d]->ptr_arr realloc\n", i);
			obj_table_g[i].ptr_arr_size *= 2;
			obj_table_g[i].ptr_arr = realloc(obj_table_g[i].ptr_arr,
					obj_table_g[i].ptr_arr_size * sizeof(kek_obj_t **));
			assert(obj_table_g[i].ptr_arr);
		}

		obj_table_g[i].ptr_arr[obj_table_g[i].ptr_arr_cnt] = objptr;

		return (i);
	}
}

/******************************************************************************/
/* gc */

int gc_ticks_g = GC_TICKS_DEFAULT;

void gc_rootset_print(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	assert(IS_PTR(obj));

	switch (obj->h.t) {
	case KEK_INT:
		vm_debug(DBG_GC, "rootset: int %d\n", obj->k_int.value);
		break;
	case KEK_STR:
		vm_debug(DBG_GC, "rootset: str %s\n", &(obj->k_str.string));
		break;
	case KEK_SYM:
		vm_debug(DBG_GC, "rootset: sym %s\n", &(obj->k_sym.symbol));
		break;
	case KEK_ARR:
		vm_debug(DBG_GC, "rootset: arr\n");
		break;
	case KEK_EXINFO:
		vm_debug(DBG_GC, "rootset: exinfo\n");
		break;
	case KEK_EXPT:
		vm_debug(DBG_GC, "rootset: expt\n");
		break;
	case KEK_FILE:
		vm_debug(DBG_GC, "rootset: file\n");
		break;
	case KEK_TERM:
		vm_debug(DBG_GC, "rootset: term\n");
		break;
	case KEK_UDO:
		vm_debug(DBG_GC, "rootset: udo\n");
		break;
	case KEK_CLASS:
		vm_debug(DBG_GC, "rootset: class %s\n", ((class_t *) (obj))->name);
		break;
	default:
		vm_debug(DBG_GC, "rootset: ??? obj=%p\n", obj);
		break;
	}
}

void gc_rootset(void (*fn)(kek_obj_t **)) {
	int i;
	uint32_t j;
	uint32_t k;
	kek_obj_t *obj_ptr;

	/* static variables of objects */
	vm_debug(DBG_GC, "rootset: static vars of objs --\n");
	for (j = 0; j < classes_cnt_g; j++) {
		for (k = 0; k < classes_g[j].syms_static_cnt; k++) {
			if (classes_g[j].syms_static[k].value != NULL
					&& IS_PTR(classes_g[j].syms_static[k].value)) {
				(*fn)(&classes_g[j].syms_static[k].value);
			}
		}
	}

	/* stack objects */
	vm_debug(DBG_GC, "rootset: objs on the stack --\n");
	for (i = sp_g - 1; i >= 0; i--) {
		if (stack_g[i] == NULL && IS_PTR(stack_g[i])) {
			(*fn)(&stack_g[i]);
		}
	}
}

/* this function will be called every X ticks */
void gc() {
	vm_debug(DBG_GC, "---------------- gc() ----------------\n");

	//gc_rootset(gc_rootset_print);
	//obj_table_print();
}

void gc_init() {
	switch (gc_type_g) {
	case GC_NONE:
		segments_g = mem_segment_init(SEGMENT_SIZE);
		break;
	case GC_NEW:
	case GC_GEN:
		gc_cheney_init();
		break;
	default:
		assert(0 && "unknown gc_type");
		break;
	}
}

void gc_free() {
	segment_t *ptr;
	segment_t *next;

	switch (gc_type_g) {
	case GC_NONE:
		next = segments_g;
		while (next != NULL) {
			ptr = next;
			next = next->next;
			free(ptr);
		}
		break;
	case GC_NEW:
	case GC_GEN:
		gc_cheney_free();
		break;
	default:
		assert(0 && "unknown gc_type");
		break;
	}

}

void *gc_obj_malloc(type_t type, class_t *cls, size_t size) {
	void *ptr;

	switch (gc_type_g) {
	case GC_NONE:
		ptr = mem_obj_malloc(type, cls, size);
		break;
	case GC_NEW:
	case GC_GEN:
		ptr = gc_cheney_malloc(type, cls, size);
		break;
	default:
		assert(0 && "unknown gc_type");
		break;
	}

	return (ptr);
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
	vm_debug(DBG_MEM, "== alloc_string length=%d\n", length);
	return (mem_obj_malloc(KEK_STR, str_class, sizeof(kek_string_t) + length));
}

kek_obj_t * alloc_integer(void) {
	vm_debug(DBG_MEM, "== alloc_integer\n");
	return (mem_obj_malloc(KEK_INT, NULL, sizeof(kek_int_t)));
}

kek_obj_t * alloc_udo(class_t * udo_class) {
	/* Calloc is used to avoid valgrind warnings about
	 * invalid reads/writes to uninitialized memory */
	uint32_t syms_cnt = udo_class->syms_instance_cnt;
	class_t * p_cls = udo_class;
	int var_offset = 0;
	kek_obj_t * ret;

	vm_debug(DBG_VM, "%s: syms_cnt before = %u\n", udo_class->name, syms_cnt);
	while (p_cls->syms_instance_cnt == 0 && p_cls->parent) {
		p_cls = p_cls->parent;
	}
// Add symbols from parents
	syms_cnt += p_cls->syms_instance[0].addr + 1; // TODO: we should do this better

	vm_debug(DBG_VM, "%s: syms_cnt after = %u\n", udo_class->name, syms_cnt);

// When parant is not udo, we need to set var_offset
	if (udo_class->parent && udo_class->parent->allocator != alloc_udo) {
		var_offset = (int64_t) udo_class->parent->allocator(NULL);
		syms_cnt += var_offset;
	}

	ret = mem_obj_calloc(KEK_UDO, udo_class,
			sizeof(kek_udo_t)
					+ (syms_cnt ? syms_cnt - 1 : 0) * sizeof(kek_obj_t), 1);
	ret->k_udo.var_offset = var_offset;

	return (ret);
}

kek_obj_t * alloc_file(class_t * file_class) {
	if (file_class == NULL) { // Helper for alloc_udo
		return (kek_obj_t*) 1; // Returns desired var_offset for derived object
	}
	return (mem_obj_malloc(KEK_FILE, file_class, sizeof(kek_file_t)));
}

kek_obj_t * alloc_term(class_t * term_class) {
	return (mem_obj_malloc(KEK_TERM, term_class, sizeof(kek_term_t)));
}

kek_obj_t * alloc_exception(class_t * expt_class) {
	if (expt_class == NULL) { // Helper for alloc_udo
		return (kek_obj_t*) 1; // Returns desired var_offset for derived object
	}
	return (mem_obj_malloc(KEK_EXPT, expt_class, sizeof(kek_except_t)));
}
