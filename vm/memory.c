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
gc_carrlist_t *gc_carrlist_root_g = NULL;

void gc_obj_add(kek_obj_t *obj, size_t size) {
	gc_obj_t *go;

	if (gc_type_g != GC_NONE) {
		return;
	}

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
//		free(obj->obj->k_arr.elems);
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

	if (gc_type_g != GC_NONE) {
		return;
	}

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
void *to_space_free_g;
size_t to_space_size_g;
void *alloc_ptr_g;
void *scan_ptr_g;

bool gc_cheney_ptr_in_space(void *segment_data, void *ptr, size_t size) {
	vm_debug(DBG_MEM, "gc_cheney_ptr_in_from_space:\n"
			" (segment_data start) %lu\n"
			" (          ptr from) %lu\n"
			" (            ptr to) %lu\n"
			" (segment_data end  ) %lu\n", //
			(ptruint_t) segment_data, //
			(ptruint_t) ptr, //
			(ptruint_t) ((uint8_t *) ptr + size), //
			(ptruint_t) ((uint8_t *) segment_data + NEW_SEGMENT_SIZE));

	return ((ptruint_t) segment_data <= (ptruint_t) ptr && //
			(ptruint_t) ((uint8_t *) ptr + size)
					< (ptruint_t) ((uint8_t *) segment_data + NEW_SEGMENT_SIZE));
}

bool gc_cheney_ptr_in_from_space(void *ptr, size_t size) {
	return (gc_cheney_ptr_in_space(segments_from_space_g->beginning, ptr, size));
}

bool gc_cheney_ptr_in_to_space(void *ptr, size_t size) {
	return (gc_cheney_ptr_in_space(segments_to_space_g->beginning, ptr, size));
}

kek_obj_t *gc_cheney_copy_obj_to_space_free(kek_obj_t *obj) {
	kek_obj_t *ret;
	size_t size;

	size = ALIGNED(vm_obj_size(obj));

	vm_debug(DBG_MEM, "gc_cheney_copy_obj_to_space_free: ptr=%p\n", obj);

	if ((ptruint_t) ((uint8_t *) to_space_free_g + size) >= //
			(ptruint_t) ((uint8_t *) segments_to_space_g->beginning
					+ NEW_SEGMENT_SIZE)) {
		vm_error("To space run out of space. Increase NEW_SEGMENT_SIZE.\n");
	}

	ret = memcpy(to_space_free_g, obj, size);
	to_space_free_g = ((uint8_t *) to_space_free_g + size);
	alloc_ptr_g = ((uint8_t *) alloc_ptr_g + size);

	return (ret);
}

void gc_cheney_copy_root_obj(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *copy;

	vm_debug(DBG_MEM, "gc_cheney_copy_roots: objptr=%p obj=%p t=%d\n", objptr,
			obj, obj->h.t);

	assert(IS_PTR(obj));
	assert(!vm_is_const(obj));

	if (obj->h.t == KEK_COPIED) {
		vm_debug(DBG_MEM, "gc_cheney_copy_roots: it's a copy, skip\n");
		*objptr = (kek_obj_t *) obj->h.cls;
		return;
	}

	copy = gc_cheney_copy_obj_to_space_free(obj);
	obj->h.t = KEK_COPIED;
	obj->h.cls = (struct _class *) copy;
	obj = copy;

	*objptr = obj;
}

void gc_cheney_copy_inner_obj(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *from_neighbor;
	kek_obj_t *to_neighbor;

	assert(IS_PTR(obj));
	assert(obj->h.t != KEK_COPIED);

	if (IS_PTR(obj) /* TODO: not in old space */) {
		vm_debug(DBG_GC, "gc_cheney_scavenge() obj found! obj=%p\n", obj);

		if (vm_is_const(obj)) {
			vm_debug(DBG_GC, "gc_cheney_scavenge() it's const\n");
			return;
		}

		from_neighbor = obj;

		if (from_neighbor->h.cls != NULL) {
			to_neighbor = (kek_obj_t *) from_neighbor->h.cls;
		} else {
			to_neighbor = gc_cheney_copy_obj_to_space_free(from_neighbor);
			from_neighbor->h.cls = (struct _class *) to_neighbor;
			from_neighbor->h.t = KEK_COPIED;
		}

		obj = to_neighbor;
	}
}

void gc_cheney_copy_neighbor_inner(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *from;
	kek_obj_t *to;

	//vm_debug(DBG_GC, "obj: %p\n", obj);

	assert(obj != NULL);
	assert(IS_PTR(obj));

	if (vm_is_const(obj)) {
		return;
	}

	from = obj;
	if (from->h.t == KEK_COPIED) {
		to = (kek_obj_t *) from->h.cls;
	} else {
		to = gc_cheney_copy_obj_to_space_free(from);
		from->h.t = KEK_COPIED;
		from->h.cls = (struct _class *) to;
	}
	obj = to;

	*objptr = obj;
}

void gc_cheney_copy_neighbor(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;

	vm_debug(DBG_GC, "gc_cheney_copy_inner_objs: objptr=%p obj=%p t=%d\n",
			objptr, obj, obj->h.t);

	assert(IS_PTR(obj));

	switch (obj->h.t) {
	case KEK_NIL:
		assert(0 && "only in cost tbl");
		break;
	case KEK_INT:
		break;
	case KEK_STR:
		break;
	case KEK_SYM:
		assert(0 && "only in cost tbl");
		break;
	case KEK_ARR: {
		kek_obj_t *arr_objs;
		int i;

		vm_debug(DBG_GC, "gc_cheney_copy_inner_objs() KEK_ARR\n");

		arr_objs = (kek_obj_t *) ((uint8_t *) obj->k_arr.elems
				- sizeof(kek_array_objs_header_t));
		assert(arr_objs->h.t == KEK_ARR_OBJS);

		vm_debug(DBG_GC,
				"gc_cheney_copy_inner_objs: before copy: obj->k_arr.elems: %p\n",
				obj->k_arr.elems);

		gc_cheney_copy_neighbor_inner(&arr_objs);
		obj->k_arr.elems = &(arr_objs->k_arr_objs.elems[0]);

		vm_debug(DBG_GC,
				"gc_cheney_copy_inner_objs: after copy: obj->k_arr.elems: %p\n",
				obj->k_arr.elems);
		vm_debug(DBG_GC, "gc_cheney_copy_inner_objs: obj->k_arr.length: %d\n",
				obj->k_arr.length);

		for (i = 0; i < obj->k_arr.length; i++) {
			kek_obj_t * el = obj->k_arr.elems[i];
			if (el != NULL && IS_PTR(el)) {
				gc_cheney_copy_neighbor_inner(&(obj->k_arr.elems[i]));
			}
		}
	}
		break;
	case KEK_ARR_OBJS:
		/* We will encounter this while scanning. However, we know
		 that elems have been already copied, so we just skip this */
		break;
	case KEK_EXINFO:
		assert(0 && "only in cost tbl");
		break;
	case KEK_EXPT:
		/* copy union _kek_obj * msg; */
		gc_cheney_copy_neighbor_inner(&(obj->k_expt.msg));
		break;
	case KEK_FILE:
		break;
	case KEK_TERM:
		// assert(0 && "this should  be in oldspace");
		/* todo: bude v oldspace (vsechno  co je v sys) */
		break;
	case KEK_UDO: {
		uint32_t i;
		for (i = 0; i < obj->h.cls->syms_instance_cnt; i++) {
			gc_cheney_copy_neighbor_inner(&(obj->k_udo.inst_var[i]));
		}
	}
		break;
	case KEK_CLASS:
		assert(0);
		break;
	case KEK_COPIED:
		assert(0);
		break;
	default:
		vm_error("Unknown obj->h.t=%d in gc_cheney_copy_inner_objs\n",
				obj->h.t);
		break;
	}
}

void gc_cheney_scavenge() {
	segment_t *swap_ptr;
	kek_obj_t *obj;
	kek_obj_t *obj_inner;

	uint8_t i;

	vm_debug(DBG_GC_STATS, "new space scavenge\n");

	vm_debug(DBG_GC, "gc_cheney_scavenge()\n");

	/* swap from-space to to-space */
	vm_debug(DBG_GC, "gc_cheney_scavenge() swap from- to-\n");
	swap_ptr = segments_from_space_g;
	segments_from_space_g = segments_to_space_g;
	segments_to_space_g = swap_ptr;

	/* set free and alloc ptr to the beginning of the to-space segment */
	to_space_free_g = segments_to_space_g->beginning;
	alloc_ptr_g = to_space_free_g;
	scan_ptr_g = to_space_free_g;

	/* clear space */
#if FORCE_CALLOC == 1
	memset(segments_to_space_g->beginning, 0, NEW_SEGMENT_SIZE);
#endif /* FORCE_CALLOC */

	vm_debug(DBG_GC, "gc_cheney_scavenge() copy roots BEGIN\n");
	gc_rootset(gc_cheney_copy_root_obj);
	vm_debug(DBG_GC, "gc_cheney_scavenge() copy roots END\n");

	vm_debug(DBG_GC, "gc_cheney_scavenge() copy inner objs BEGIN\n");
	while ((ptruint_t) scan_ptr_g < (ptruint_t) alloc_ptr_g) {
		obj = (kek_obj_t *) scan_ptr_g;
		assert(obj != NULL);
		assert(IS_PTR(obj));
		scan_ptr_g = ((uint8_t *) scan_ptr_g) + ALIGNED(vm_obj_size(obj));
		gc_cheney_copy_neighbor(&obj);
	}
	vm_debug(DBG_GC, "gc_cheney_scavenge() copy inner objs END\n");

	vm_debug(DBG_GC, "gc_cheney_scavenge() end "
			"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
}

void gc_cheney_init() {
	vm_debug(DBG_GC, "gc_cheney_init()\n");
	segments_from_space_g = mem_segment_init(NEW_SEGMENT_SIZE);
	segments_to_space_g = mem_segment_init(NEW_SEGMENT_SIZE);

#if FORCE_CALLOC == 1
	memset(segments_from_space_g->beginning, 0, NEW_SEGMENT_SIZE);
	memset(segments_to_space_g->beginning, 0, NEW_SEGMENT_SIZE);
#endif /* FORCE_CALLOC */

	to_space_free_g = segments_to_space_g->end;
	to_space_size_g = 0;
}

void gc_cheney_free() {
	free(segments_from_space_g);
	free(segments_to_space_g);
}

void *gc_cheney_malloc(type_t type, class_t *cls, size_t size) {
	void *ptr;
	segment_t *new;

	size = ALIGNED(size);

	if (size >= NEW_SEGMENT_SIZE) {
		vm_error("Size %d is too big for NEW_SEGMENT_SIZE\n", size);
	}

	assert(segments_to_space_g != NULL);

	if ((ptruint_t) ((uint8_t *) to_space_free_g + size) >= //
			(ptruint_t) ((uint8_t *) segments_to_space_g->beginning
					+ NEW_SEGMENT_SIZE)) {
		vm_debug(DBG_GC, "gc_cheney_malloc: From space needs GC. "
				"##########################################################\n");
		gc_cheney_scavenge();
	}

	ptr = to_space_free_g;

	to_space_free_g = (uint8_t *) to_space_free_g + size;
	to_space_size_g += size;

	vm_debug(DBG_GC,
			"gc_cheney_malloc: size=%lu from=%p to=%p (to=%p toend=%p)\n", //
			size, ptr, to_space_free_g, segments_to_space_g,
			(uint8_t *) segments_to_space_g + NEW_SEGMENT_SIZE);

	if (!gc_cheney_ptr_in_to_space(ptr, size)) {
		vm_error("cheney_malloc: ptr=%p is not in to-space\n", ptr);
	}

#ifdef FORCE_CALLOC
	memset(ptr, 0, size);
#endif /* FORCE_CALLOC */

	((kek_obj_t *) ptr)->h.t = type;
	((kek_obj_t *) ptr)->h.cls = cls;

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

#if FORCE_CALLOC == 1
	s = calloc(sizeof(segment_t) + size, 1);
#else /* FORCE_CALLOC */
	s = malloc(sizeof(segment_t) + size);
#endif /* FORCE_CALLOC */

	assert(s);

	vm_debug(DBG_MEM, "mem_segment_init(size=%u), segment ptr=%p\n", size, s);

	s->size = size;
	s->used = 0;
	s->next = NULL;
	s->beginning = s + 1; // Same as (uint8_t*) s + sizeof(segment_t)
	s->end = s->beginning;

	/* in the segment_t, there is 1 data. but the s->beginning points to the
	 * 0th data.
	 */
	/*assert((uint8_t *) s + sizeof(segment_t) + (size - 1) * OBJ_ALIGN == //
	 (uint8_t *) s->beginning + size * OBJ_ALIGN);*/

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

	if (gc_type_g != GC_NONE) {
		int *x = (int *) 42;
		*x = 666; // WTF?
	}

	assert(segments_g != NULL);

	if (segments_g->used + size > segments_g->size) {
		vm_debug(DBG_MEM, "We need to allocate a new segment.\n");
		new = mem_segment_init(SEGMENT_SIZE);
		new->next = segments_g;
		segments_g = new;
	}

	ptr = segments_g->end;
	segments_g->used += size;
	// I don't know why but if I don't add 8, I get segfault...
	segments_g->end = (uint8_t *) segments_g->end + size;

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
			obj_table_g[i].ptr_arr_size =
			OBJ_TABLE_PTR_ARR_DEFAULT_SIZE;
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
	gc_carrlist_t *carr;

	/* static variables of objects */
	vm_debug(DBG_GC, "rootset: static vars of objs --\n");
	for (j = 0; j < classes_cnt_g; j++) {
		for (k = 0; k < classes_g[j].syms_static_cnt; k++) {

			vm_debug(DBG_GC, "rootset: static vars[%d] has ptr=%p --\n", k,
					&classes_g[j].syms_static[k].value);

			if (classes_g[j].syms_static[k].value != NULL
					&& IS_PTR(classes_g[j].syms_static[k].value)) {
				(*fn)(&classes_g[j].syms_static[k].value);
			}
		}
	}

	/* arrays from const table */
	for (carr = gc_carrlist_root_g; carr; carr = carr->next) {
		(*fn)((kek_obj_t **) &(carr->arr));
	}

	/* stack objects */
	vm_debug(DBG_GC, "rootset: objs on the stack --\n");
	for (i = sp_g - 1; i >= 0; i--) {
		vm_debug(DBG_GC, "rootset stack[%d]\n", i);
		if (stack_g[i] != NULL && IS_PTR(stack_g[i])) {

			if (((kek_obj_t *) stack_g[i])->h.t == KEK_CLASS) {
				vm_debug(DBG_GC, "rootset: ignoring class\n");
				continue;
			}

			if (((kek_obj_t *) stack_g[i])->h.t == KEK_STACK) {
				vm_debug(DBG_GC, "rootset: ignoring stack reference\n");
				continue;
			}

			if (((kek_obj_t *) stack_g[i])->h.t == KEK_NIL) {
				assert(vm_is_const((kek_obj_t * ) stack_g[i]));
			}

			if (vm_is_const(((kek_obj_t *) stack_g[i]))) {
				vm_debug(DBG_GC, "rootset: ignoring const\n");
				continue;
			}
			gc_rootset_print(&stack_g[i]);

			(*fn)(&stack_g[i]);
		}
	}
}

/* this function will be called every X ticks */
void gc() {
	vm_debug(DBG_GC_STATS, "%6lu, used %.2lf %%\n", ticks_g, gc_remaining());
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
	gc_carrlist_t *cal;

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

	// Free constant array list
	cal = gc_carrlist_root_g;
	while (cal != NULL) {
		gc_carrlist_t *next = cal->next;
		free(cal);
		cal = next;
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

//if ((ptruint_t)((uint8_t *) to_space_free_g + size) >= //
//		(ptruint_t)((uint8_t *) segments_to_space_g + NEW_SEGMENT_SIZE)) {

double gc_remaining(void) {
	if (gc_type_g == GC_NONE)
		return 0;

	return ((double) ((ptruint_t) to_space_free_g
			- (ptruint_t) segments_to_space_g->beginning)
			/ ((double) NEW_SEGMENT_SIZE));
}

/******************************************************************************/

kek_obj_t * alloc_array(class_t * arr_class) {
	return (gc_obj_malloc(KEK_ARR, arr_class, sizeof(kek_array_t)));
}

kek_obj_t *alloc_array_objs(int items) {
	kek_obj_t * ret = (gc_obj_malloc(KEK_ARR_OBJS, NULL,
			sizeof(kek_array_objs_t) + (items - 1) * sizeof(kek_obj_t *)));
	ret->k_arr_objs.h.length = items;
	return ret;
}

void alloc_arr_elems(kek_array_t * arr) {
	arr->elems = alloc_const_arr_elems(ARR_INIT_SIZE);
	assert(arr->elems);
	arr->alloc_size = ARR_INIT_SIZE;
}

kek_obj_t **alloc_const_arr_elems(int length) {
	kek_obj_t *array_objs = alloc_array_objs(length);
	return ((kek_obj_t **) &(array_objs->k_arr_objs.elems[0]));
}

void realloc_arr_elems(kek_array_t *arr, int length) {
	kek_obj_t **new_elems;
	int i;

	vm_debug(DBG_MEM, "realloc_arr_elems\n");

	while (arr->alloc_size < length) {
		arr->alloc_size *= 2;
	}

	new_elems = alloc_const_arr_elems(arr->alloc_size);
	// TODO: we need arr pointer update
	// if realloc triggered GC.
	// This means we copy the argument arr
	// to local variable and pass its address to GC rootset

	for (i = 0; i < arr->length; i++) {
		new_elems[i] = arr->elems[i];
	}

#if FORCE_CALLOC == 1
	for (; i < arr->alloc_size; i++) {
		new_elems[i] = NULL;
	}
#endif /* FORCE_CALLOC == 1 */

	arr->elems = new_elems;
	//arr->length = length;

	/* NOTE: the old elems will cleanup GC */
}

kek_obj_t * alloc_string(class_t * str_class, int length) {
	vm_debug(DBG_MEM, "== alloc_string length=%d\n", length);
	return (gc_obj_malloc(KEK_STR, str_class, sizeof(kek_string_t) + length));
}

kek_obj_t * alloc_integer(void) {
	vm_debug(DBG_MEM, "== alloc_integer\n");
	return (gc_obj_malloc(KEK_INT, NULL, sizeof(kek_int_t)));
}

kek_obj_t * alloc_udo(class_t * udo_class) {
	/* Calloc is used to avoid valgrind warnings about
	 * invalid reads/writes to uninitialized memory */
	int syms_cnt = udo_class->total_syms_instance_cnt;
	int var_offset = udo_class->syms_instance_offset;
	/* total_size is the number of elements in inst_var */
	int total_size = syms_cnt + var_offset;
	kek_obj_t * ret;

	ret = gc_obj_malloc(KEK_UDO, udo_class,
			sizeof(kek_udo_t)
					+ (total_size ? total_size - 1 : 0) * sizeof(kek_obj_t));
	ret->k_udo.var_offset = var_offset;

	return (ret);
}

kek_obj_t * alloc_file(class_t * file_class) {
	if (file_class == NULL) { // Helper for alloc_udo
		return (kek_obj_t*) 1; // Returns desired var_offset for derived object
	}
	return (gc_obj_malloc(KEK_FILE, file_class, sizeof(kek_file_t)));
}

kek_obj_t * alloc_term(class_t * term_class) {
	return (gc_obj_malloc(KEK_TERM, term_class, sizeof(kek_term_t)));
}

kek_obj_t * alloc_exception(class_t * expt_class) {
	if (expt_class == NULL) { // Helper for alloc_udo
		return (kek_obj_t*) 1; // Returns desired var_offset for derived object
	}
	return (gc_obj_malloc(KEK_EXPT, expt_class, sizeof(kek_except_t)));
}
