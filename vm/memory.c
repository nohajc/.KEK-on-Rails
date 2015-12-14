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
#include "k_array.h"

gc_obj_t *gc_obj_g = NULL;
gc_obj_t *gc_obj_root_g = NULL;
gc_carrlist_t *gc_carrlist_root_g = NULL;

void gc_obj_add(kek_obj_t *obj, size_t size) {
	gc_obj_t *go;

	size = ALIGNED(size);

#ifndef JUST_USE_MALLOC
	if (gc_type_g != GC_NONE) {
		return;
	}
#endif

	vm_debug(DBG_GC, "gc_obj_add objptr=%p\n", (void*) obj);

	if (obj == NULL) {
		vm_error("gc_obj_add obj is null\n");
	}

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

void gc_delete_all() {
	gc_obj_t *gcptr = gc_obj_root_g;
	gc_obj_t *gcptr_next;

#ifndef JUST_USE_MALLOC
	if (gc_type_g != GC_NONE) {
		return;
	}
#endif

	vm_debug(DBG_GC, "gc_delete_all\n");

	while (gcptr != NULL) {
		vm_debug(DBG_GC, "gc_delete_all gcptr=%p (%lu)\n", gcptr,
				(ptruint_t) gcptr);
		gcptr_next = gcptr->next;
		free(gcptr->obj);
		free(gcptr);
		gcptr = gcptr_next;
	}
}

/******************************************************************************/
/* Cheney */
/* https://en.wikipedia.org/wiki/Cheney's_algorithm */
/* http://jayconrod.com/posts/55/a-tour-of-v8-garbage-collection */

/* global variables */
void *segments_from_space_g = NULL;
void *segments_to_space_g = NULL;
void *to_space_free_g;
size_t to_space_size_g;
void *scan_ptr_g;
uint32_t gc_cheney_iteration_t;

bool gc_cheney_ptr_in_space(void *segment_data, void *ptr, size_t size) {
	size = ALIGNED(size);
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
	size = ALIGNED(size);
	return (gc_cheney_ptr_in_space(segments_from_space_g, ptr, size));
}

bool gc_cheney_ptr_in_to_space(void *ptr, size_t size) {
	size = ALIGNED(size);
	return (gc_cheney_ptr_in_space(segments_to_space_g, ptr, size));
}

kek_obj_t *gc_cheney_copy_obj_to_space_free(kek_obj_t *obj) {
	kek_obj_t *ret;
	size_t size;

	size = ALIGNED(vm_obj_size(obj));

	vm_debug(DBG_MEM, "gc_cheney_copy_obj_to_space_free: ptr=%p\n", obj);
	assert(OBJ_TYPE_CHECK(obj));

	if ((ptruint_t) ((uint8_t *) to_space_free_g + size) >= //
			(ptruint_t) ((uint8_t *) segments_to_space_g + NEW_SEGMENT_SIZE)) {
		vm_error("To space run out of space. Increase NEW_SEGMENT_SIZE.\n");
	}

	ret = memcpy(to_space_free_g, obj, size);
	to_space_free_g = ((uint8_t *) to_space_free_g) + size;

	return (ret);
}

void gc_cheney_copy_root_obj(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *copy;

	vm_debug(DBG_GC, "gc_cheney_copy_roots: objptr=%p obj=%p t=%d\n^^^\n\n",
			objptr, obj, obj->h.t);

	if (obj->h.t == KEK_ARR) {
		vm_debug(DBG_GC, "yes, this is an array. alloc_size=%d\n\n",
				obj->k_arr.alloc_size);
	}

	assert(IS_PTR(obj));
	//assert(!vm_is_const(obj));
	if (vm_is_const(obj)) {
		assert_failed: //
		return;
	}

	if (obj->h.t == KEK_COPIED) {
		vm_debug(DBG_GC, "gc_cheney_copy_roots: it's a copy, skip\n");
		*objptr = (kek_obj_t *) obj->h.cls;
		return;
	}

	if (gc_cheney_ptr_in_to_space(obj, sizeof(header_t))) {
		return;
	}

	if (gc_type_g == GC_GEN) {
		switch (obj->h.state) {
		case OBJ_1ST_GEN_YOUNG:
			obj->h.state = OBJ_2ND_GEN_YOUNG;
			break;
		case OBJ_2ND_GEN_YOUNG:
			vm_debug(DBG_GC_STATS, "Moving obj=%p to old space.\n", obj);

			/* If the object survived twice in the old space, move it to the
			 * old space. This function will memcpy the object, set its
			 * state to WHITE and memcpy all its neighbors recursively.
			 */
			gc_os_rec_cpy_neighbors(objptr);

			/* RETURN */
			return;
		default:
			vm_error("Invalid obj state=%d\n", obj->h.state);
			break;
		}
	}

	copy = gc_cheney_copy_obj_to_space_free(obj);
	assert(IS_PTR(copy));
	assert(OBJ_TYPE_CHECK(copy));
	assert(copy->h.t == obj->h.t);

	obj->h.t = KEK_COPIED;
	obj->h.cls = (struct _class *) copy;

	obj = copy;

	*objptr = obj;
}

void gc_cheney_copy_neighbor_inner(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	kek_obj_t *from;
	kek_obj_t *to;

	//vm_debug(DBG_GC, "obj: %p\n", obj);

	assert(obj != NULL);
	assert(IS_PTR(obj));
	assert(OBJ_TYPE_CHECK(obj));

	if (vm_is_const(obj)) {
		vm_debug(DBG_GC, "gc_cheney_copy_neighbor_inner: obj=%p is const!\n",
				(void *) obj);
		return;
	}

	from = obj;
	if (from->h.t == KEK_COPIED) {
		to = (kek_obj_t *) from->h.cls;
	} else if (gc_cheney_ptr_in_to_space(obj, sizeof(header_t))) {
		return;
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
		break;
	case KEK_ARR: {
		kek_array_objs_t *arr_objs;
		int i;

		vm_debug(DBG_GC, "+++++ KEK_ARR +++++\n");
		vm_debug(DBG_GC, "KA obj=%p\n", obj);
		vm_debug(DBG_GC, "KA obj->k_arr.elems=%p\n", obj->k_arr.elems);

		if (obj->k_arr.elems == NULL) {
			break;
		}
		arr_objs = KEK_ARR_OBJS(obj);

		/*
		 // arr_objs may already be copied!
		 // This debug message could access invalid memory.
		 for (i = 0; i < arr_objs->h.length; i++) {
		 if (IS_UDO(arr_objs->elems[i])) {
		 if (IS_INT(arr_objs->elems[i]->k_udo.inst_var[0])) {
		 vm_debug(DBG_GC, "i=%d WILL CPY %p. val=%d\n^^^^\n",i,
		 arr_objs->elems[i], //
		 INT_VAL(arr_objs->elems[i]->k_udo.inst_var[0]));
		 }
		 }
		 }*/

		/* this will copy the structure with the pointers to the objs */
		gc_cheney_copy_neighbor_inner((kek_obj_t **) &arr_objs);

		obj->k_arr.elems = &(arr_objs->elems[0]);

		vm_debug(DBG_GC,
				"gc_cheney_copy_inner_objs: after copy: obj->k_arr.elems: %p\n",
				obj->k_arr.elems);
		vm_debug(DBG_GC, "gc_cheney_copy_inner_objs: obj->k_arr.length: %d\n",
				obj->k_arr.length);
	}
		break;
	case KEK_ARR_OBJS: {
		int i;

		for (i = 0; i < obj->k_arr_objs.h.length; i++) {
			kek_obj_t * el = obj->k_arr_objs.elems[i];

			vm_debug(DBG_GC, "==: obj->k_arr_objs.h.length: %d, i=%d, el=%p\n",
					obj->k_arr_objs.h.length, i, el);

			if (IS_NIL(el)) {
				vm_debug(DBG_GC, "el is nil %d\n");
			}

			if (IS_INT(el)) {
				vm_debug(DBG_GC, "el is int %d\n", INT_VAL(el));
			}

			vm_assert(el != NULL, "el %d is NULL\n", i);
			if (IS_PTR(el)) {

				if (obj->k_arr_objs.elems[i]->h.t == KEK_UDO) {
					vm_debug(DBG_GC, "XXX\narr obj is udo of %s\n",
							obj->k_arr_objs.elems[i]->k_udo.h.cls->name);

					uint32_t x;
					for (x = 0;
							x < //
									obj->k_arr_objs.elems[i]->k_udo.h.cls->syms_instance_cnt;
							x++) {
						if (IS_INT(
								obj->k_arr_objs.elems[i]->k_udo.inst_var[x])) {
							vm_debug(DBG_GC, "inst val=%d\n",
									obj->k_arr_objs.elems[i]->k_udo.inst_var[x]);
						} else {
							vm_debug(DBG_GC, "inst val=?\n");
						}
					}

				}

				gc_cheney_copy_neighbor_inner(&(obj->k_arr_objs.elems[i]));
			}
		}
		break;
	}
	case KEK_EXINFO:
		assert(0 && "only in const tbl");
		break;
	case KEK_EXPT: {
		/* copy union _kek_obj * msg; */
		kek_obj_t * msg = obj->k_expt.msg;
		if (msg != NULL && IS_PTR(msg)) {
			gc_cheney_copy_neighbor_inner(&(obj->k_expt.msg));
		}
		break;
	}
	case KEK_FILE:
		break;
	case KEK_TERM:
		// assert(0 && "this should  be in oldspace");
		/* todo: bude v oldspace (vsechno  co je v sys) */
		break;
	case KEK_UDO: {
		int i;
		bool verbose = false;
		int total_size = obj->h.cls->total_syms_instance_cnt
				+ obj->h.cls->syms_instance_offset;
		if (!strcmp(obj->h.cls->name, "Reader")) {
			cp_reader: //
			verbose = true;
		}
		for (i = 0; i < total_size; i++) {
			kek_obj_t * var = obj->k_udo.inst_var[i];
			if (var != NULL && IS_PTR(var)) {
				if (verbose) {
					vm_debug(DBG_BC,
							"Copying inst_var[%d], type = %d, ptr = %p, ", i,
							var->h.t, var);
				}
				gc_cheney_copy_neighbor_inner(&(obj->k_udo.inst_var[i]));
				if (verbose) {
					vm_debug(DBG_BC, "new_ptr = %p.\n", obj->k_udo.inst_var[i]);
				}
			}
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

	gc_cheney_iteration_t++;
	vm_debug(DBG_GC_STATS, "new space scavenge\n");

	vm_debug(DBG_GC, "gc_cheney_scavenge()\n");

	/* swap from-space to to-space */
	vm_debug(DBG_GC, "gc_cheney_scavenge() swap from- to-\n");
	swap_ptr = segments_from_space_g;
	segments_from_space_g = segments_to_space_g;
	segments_to_space_g = swap_ptr;

	/* set free and alloc ptr to the beginning of the to-space segment */
	to_space_free_g = segments_to_space_g;
	scan_ptr_g = to_space_free_g;

	/* clear space */
#if FORCE_CALLOC == 1
	memset(segments_to_space_g->beginning, 0, NEW_SEGMENT_SIZE);
#endif /* FORCE_CALLOC */

	vm_debug(DBG_GC, "gc_cheney_scavenge() copy roots BEGIN\n");
	gc_rootset(gc_cheney_copy_root_obj);
	vm_debug(DBG_GC, "gc_cheney_scavenge() copy roots END\n");

	vm_debug(DBG_GC, "gc_cheney_scavenge() copy inner objs BEGIN\n");
	while ((ptruint_t) scan_ptr_g < (ptruint_t) to_space_free_g) {
		obj = (kek_obj_t *) scan_ptr_g;

		vm_debug(DBG_GC, "gc_cheney_scavenge() scaned obj=%p\n", obj);

		assert(obj != NULL);
		assert(IS_PTR(obj));
		assert(OBJ_TYPE_CHECK(obj));

		scan_ptr_g = ((uint8_t *) scan_ptr_g) + ALIGNED(vm_obj_size(obj));

		gc_cheney_copy_neighbor(&obj);
	}

	vm_debug(DBG_GC, "gc_cheney_scavenge() end "
			"&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
}

void gc_cheney_init() {
	vm_debug(DBG_GC, "gc_cheney_init()\n");

	segments_from_space_g = malloc(
	NEW_SEGMENT_SIZE * OBJ_ALIGN * sizeof(uint8_t));

	assert(segments_from_space_g);

	segments_to_space_g = malloc(
	NEW_SEGMENT_SIZE * OBJ_ALIGN * sizeof(uint8_t));

	assert(segments_to_space_g);

#if FORCE_CALLOC == 1
	(void) memset(segments_from_space_g, 0, NEW_SEGMENT_SIZE);
	(void)memset(segments_to_space_g, 0, NEW_SEGMENT_SIZE);
#endif /* FORCE_CALLOC */

	to_space_free_g = segments_to_space_g;
	to_space_size_g = 0;
	gc_cheney_iteration_t = 0;
}

void gc_cheney_free() {

	vm_debug(DBG_GC_STATS, "gc_cheney_iteration_t = %d\n",
			gc_cheney_iteration_t);

	free(segments_from_space_g);
	free(segments_to_space_g);
}

bool gc_cheney_can_malloc(size_t size) {
	size = ALIGNED(size);
	return ((ptruint_t) ((uint8_t *) to_space_free_g + size) < //
			(ptruint_t) ((uint8_t *) segments_to_space_g + NEW_SEGMENT_SIZE));
}

uint32_t last_id_g = 0;

void *gc_cheney_malloc(type_t type, class_t *cls, size_t size) {
	void *ptr;
	segment_t *new;

	size = ALIGNED(size);

	if (size >= NEW_SEGMENT_SIZE) {
		vm_error("Size %d is too big for NEW_SEGMENT_SIZE\n", size);
	}

	assert(segments_to_space_g != NULL);

	if (!gc_cheney_can_malloc(size)) {
		vm_debug(DBG_GC, "gc_cheney_malloc: From space needs GC. "
				"##########################################################\n");
		gc_cheney_scavenge();

		if (!gc_cheney_can_malloc(size)) {
			vm_error("No space in NEW_SPACE. Increase it please.\n");
		}
	}

	ptr = to_space_free_g;

	to_space_free_g = (uint8_t *) to_space_free_g + size;
	to_space_size_g += size;

	vm_debug(DBG_GC,
			"gc_cheney_malloc: size=%lu from=%p to=%p (to=%p toend=%p)\n", //
			size, ptr, to_space_free_g, segments_to_space_g,
			(uint8_t *) segments_to_space_g + NEW_SEGMENT_SIZE);

	if (!gc_cheney_ptr_in_to_space(ptr, size)) {
		vm_error("cheney_malloc: ptr=%p size=%lu is not in to-space\n", ptr,
				size);
	}

#ifdef FORCE_CALLOC
	memset(ptr, 0, size);
#endif /* FORCE_CALLOC */

	((kek_obj_t *) ptr)->h.t = type;
	((kek_obj_t *) ptr)->h.cls = cls;
	((kek_obj_t *) ptr)->h.id = ++last_id_g;
	((kek_obj_t *) ptr)->h.state = OBJ_1ST_GEN_YOUNG;

	return (ptr);
}

void *gc_cheney_calloc(type_t type, class_t *cls, size_t size) {
	void *ptr;
	size = ALIGNED(size);
	ptr = gc_cheney_malloc(type, cls, size);
	memset(ptr, 0, size);

	return (ptr);
}

/******************************************************************************/
/* gc_rootset */

gc_rootset_t *gc_rootset_g;
uint32_t gc_rootset_last_uid_g;

uint32_t gc_rootset_add(kek_obj_t **obj) {
	gc_rootset_t *grnew;

	if (gc_type_g == GC_NONE) {
		return (0);
	}

	assert(obj != NULL);

	grnew = malloc(sizeof(gc_rootset_t));
	assert(grnew);

	grnew->next = NULL;
	grnew->obj = obj;
	grnew->uid = ++gc_rootset_last_uid_g;

	if (gc_rootset_g == NULL) {
		gc_rootset_g = grnew;
	} else {
		grnew->next = gc_rootset_g;
		gc_rootset_g = grnew;
	}

	return (gc_rootset_last_uid_g);
}

void gc_rootset_remove_id(uint32_t id) {
	if (gc_type_g == GC_NONE) {
		return;
	}

	gc_rootset_t *gr;
	gc_rootset_t *grptr;

	assert(gc_rootset_g != NULL);
	vm_debug(DBG_GC, "gc_rootset next %p\n", gc_rootset_g->next);

	for (gr = gc_rootset_g; gr; gr = gr->next) {
		vm_debug(DBG_GC, "gr= %p\n", gr);
		vm_debug(DBG_GC, "grnext= %p\n", gr->next);

		if (gr->uid == id) {
			if ((grptr = gr->next) != NULL) {
				gr->next = grptr->next;
				gr->obj = grptr->obj;
				gr->uid = grptr->uid;
				free(grptr);
				grptr = NULL;
			} else {
				free(gr);

				if (gc_rootset_g == gr) {
					gc_rootset_g = NULL;
				}

				gr = NULL;
				break;
			}
		}
	}
}

void gc_rootset_init(void) {
	if (gc_type_g == GC_NONE) {
		return;
	}

	gc_rootset_g = NULL;
	gc_rootset_last_uid_g = 0;
}

void gc_rootset_free(void) {
	if (gc_type_g == GC_NONE) {
		return;
	}

	gc_rootset_t *gr;
	gc_rootset_t *grptr;

	for (gr = gc_rootset_g; gr; grptr = gr, gr = gr->next, free(grptr))
		;
}

/******************************************************************************/
/* memory managment */

segment_t *segments_g = NULL;

segment_t *mem_segment_init(size_t size) {
	segment_t *s;

	size = ALIGNED(size);

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
	segment_t *new_seg;

	size = ALIGNED(size);

	if (gc_type_g != GC_NONE) {
		vm_error("mem_segment when gc is not none\n");
	}

	assert(segments_g != NULL);

	if (segments_g->used + size > segments_g->size) {
		vm_error("no segment realloc pls. make it bigger\n");

		vm_debug(DBG_MEM, "We need to allocate a new segment.\n");
		if (size > SEGMENT_SIZE) {
			new_seg = mem_segment_init(size);
		} else {
			new_seg = mem_segment_init(SEGMENT_SIZE);
		}
		new_seg->next = segments_g;
		segments_g = new_seg;
	}

	ptr = segments_g->end;
	segments_g->used += size;
	segments_g->end = ((uint8_t *) segments_g->end) + size;

	vm_debug(DBG_MEM, "after: end=\t%p (%lu)\n", segments_g->end,
			segments_g->end);

	return (ptr);
}

void *mem_segment_calloc(size_t size) {
	void *ptr;

	size = ALIGNED(size);
	ptr = mem_segment_malloc(size);
	memset(ptr, 0, size);

	return (ptr);
}

/* (c)allocate memory for an object, set its header and return a pointer */
inline void *mem_obj_malloc(type_t type, class_t *cls, size_t size) {
	kek_obj_t *obj;

	size = ALIGNED(size);
#ifndef JUST_USE_MALLOC
	obj = mem_segment_malloc(size);
#else
	obj = malloc(size);
	gc_obj_add(obj, size);
#endif
	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;

	return (obj);
}

inline void *mem_obj_calloc(type_t type, class_t *cls, size_t num, size_t size) {
	kek_obj_t *obj;

	size = ALIGNED(size);
#ifndef JUST_USE_MALLOC
	obj = mem_segment_calloc(num * size);
#else
	obj = calloc(num, size);
	gc_obj_add(obj, num * size);
#endif

	assert(obj);

	obj->h.t = type;
	obj->h.cls = cls;

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
			obj_table_g[i].state = OBJ_2ND_GEN_YOUNG;

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
	obj_table_g[obj_table_size_g / 2].state = OBJ_2ND_GEN_YOUNG;

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
	for (j = 0; j < classes_cnt_g; j++) {
		for (k = 0; k < classes_g[j].syms_static_cnt; k++) {
			if (classes_g[j].syms_static[k].value != NULL
					&& IS_PTR(classes_g[j].syms_static[k].value)) {

				vm_debug(DBG_GC,
						"rootset: static vars[%d], ptr=%p, name=\"%s\"\n", k,
						&classes_g[j].syms_static[k].value,
						classes_g[j].syms_static[k].name);

				(*fn)(&classes_g[j].syms_static[k].value);
			}
		}
	}

	/* gc_rootset */
	gc_rootset_t *gr;
	for (gr = gc_rootset_g; gr; gr = gr->next) {

		vm_debug(DBG_GC, "rootset: gr=%p obj=%p\n", gr, gr->obj);

		assert(gr->obj != NULL);

		vm_debug(DBG_GC, "rootset: (LL)gc_rootset[%d], objptr=%p obj=%p, "
				"type=%d\n", j, gr->obj, *(gr->obj),
				((kek_obj_t *) *(gr->obj))->h.t);

		(*fn)(gr->obj);
	}

	/* arrays from const table */
	for (carr = gc_carrlist_root_g; carr; carr = carr->next) {
		vm_debug(DBG_GC, "rootset: carr, obj=%p\n", carr->arr);
		kek_array_objs_t * arr_objs = KEK_ARR_OBJS(carr->arr);
		(*fn)((kek_obj_t **) &arr_objs);
		carr->arr->elems = &(arr_objs->elems[0]);
	}

	/* stack objects */
	for (i = sp_g - 1; i >= 0; i--) {
		if (stack_g[i] != NULL && IS_PTR(stack_g[i])) {
			// Temporary debug msg
			/*if (stack_g[i]->h.t == KEK_COPIED) {
			 vm_error("rootset: ERROR: KEK_COPIED at stack_g[%d], obj ptr = %p.\n", i, stack_g[i]);
			 }
			 assert(stack_g[i]->h.t != KEK_COPIED);*/
			if (stack_g[i]->h.t == KEK_CLASS) {
//				vm_debug(DBG_GC, "rootset: ignoring class\n");
				continue;
			}

			if (stack_g[i]->h.t == KEK_STACK) {
//				vm_debug(DBG_GC, "rootset: ignoring stack reference\n");
				continue;
			}

			if (stack_g[i]->h.t == KEK_NIL) {
				assert(vm_is_const((kek_obj_t * ) stack_g[i]));
			}

			if (vm_is_const(stack_g[i])) {
//				vm_debug(DBG_GC, "rootset: ignoring const\n");
				continue;
			}

			vm_debug(DBG_GC, "rootset stack[%d]\n", i);
			gc_rootset_print(&stack_g[i]);
			(*fn)(&stack_g[i]);
		}
	}

	/* pointers in the old space that points to the new space are stored
	 * in the remember set */
	if (gc_type_g == GC_GEN) {
		os_remember_set_t *rsptr;

		for (rsptr = gc_os_remember_set_g; rsptr; rsptr = rsptr->next) {
			assert(IS_PTR(*(rsptr->new_obj)));
			assert(OBJ_TYPE_CHECK(*(rsptr->new_obj)));

			assert(!gc_os_is_in_old(*(rsptr->new_obj)));
			assert(gc_os_is_in_new(*(rsptr->new_obj)));

			(*fn)(rsptr->new_obj);
		}
	}
}

/* this function will be called every X ticks */
void gc() {
	vm_debug(DBG_GC_STATS, "%6lu, used %.2lf %%\n", ticks_g,
			gc_remaining() * 100);
}

void gc_init() {
	switch (gc_type_g) {
	case GC_NONE:
		segments_g = mem_segment_init(SEGMENT_SIZE);
		break;
	case GC_NEW:
	case GC_GEN:
		gc_rootset_init();
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
		gc_rootset_free();
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

	size = ALIGNED(size);

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
			- (ptruint_t) segments_to_space_g) / ((double) NEW_SEGMENT_SIZE));
}

/******************************************************************************/

kek_obj_t * alloc_array(class_t * arr_class) {
	kek_obj_t * ret = gc_obj_malloc(KEK_ARR, arr_class, sizeof(kek_array_t));
	ret->k_arr.elems = NULL;
	return (ret);
}

kek_obj_t *alloc_array_objs(int items) {
	int i;
	vm_debug(DBG_GC, "alloc_array_objs BEGIN ++++++++++++++++++++++++++++++\n");
	kek_obj_t * ret = (gc_obj_malloc(KEK_ARR_OBJS, NULL,
			sizeof(kek_array_objs_t) + (items - 1) * sizeof(kek_obj_t *)));
	for (i = 0; i < items; i++) {
		ret->k_arr_objs.elems[i] = NIL;
	}
	vm_debug(DBG_GC, "alloc_array_objs END --------------------------------\n");
	return (ret);
}

kek_obj_t **alloc_arr_elems(int size, int length) {
	vm_debug(DBG_GC, "alloc_arr_elems BEGIN +++++++++++++++++++++++++++++++\n");
	kek_obj_t *array_objs = alloc_array_objs(size);
	array_objs->k_arr_objs.h.length = length;
	vm_debug(DBG_GC, "alloc_arr_elems END ---------------------------------\n");
	return ((kek_obj_t **) &(array_objs->k_arr_objs.elems[0]));
}

kek_obj_t **alloc_const_arr_elems(int length) {
	vm_debug(DBG_GC, "alloc_const_arr_elems BEGIN +++++++++++++++++++++++++\n");
	kek_obj_t *array_objs = alloc_array_objs(length);
	array_objs->k_arr_objs.h.length = length;
	vm_debug(DBG_GC, "alloc_const_arr_elems END ---------------------------\n");
	return ((kek_obj_t **) &(array_objs->k_arr_objs.elems[0]));
}

void arr_realloc_elems(kek_array_t *arr, int length) {
	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);
	kek_obj_t **new_elems;
	int i;
	int new_size;

	vm_debug(DBG_MEM, "qq realloc_arr_elems asbef=%d len=%d arr->length=%d"
			" elemlen=%d\n", arr->alloc_size, length, arr->length,
	KEK_ARR_OBJS(arr)->h.length);

	assert(arr->elems[arr->length - 1] != NULL);

	new_size = arr->alloc_size;
	while (new_size < length) {
		//arr->alloc_size = (arr->alloc_size * 3) / 2;
		new_size = (new_size * 3) / 2;
	}

	vm_debug(DBG_MEM, "qq realloc_arr_elems then arr->alloc_size=%d \n", //
			new_size);

	new_elems = alloc_arr_elems(new_size, arr->length);

	for (i = 0; i < arr->length; i++) {
		new_elems[i] = arr->elems[i];
	}

	KEK_ARR_OBJS(arr)->h.length = arr->length;
	arr->elems = new_elems;
	arr_set_alloc_size(arr, new_size);

	gc_rootset_remove_id(id);

	/* NOTE: the old elems will cleanup GC */
}

kek_obj_t * alloc_string(class_t * str_class, int length) {
	vm_debug(DBG_MEM, "== alloc_string length=%d\n", length);
	return (gc_obj_malloc(KEK_STR, str_class, sizeof(kek_string_t) + length));
}

kek_obj_t * alloc_symbol(int length) {
	vm_debug(DBG_MEM, "== alloc_symbol length=%d\n", length);
	return (gc_obj_malloc(KEK_SYM, NULL, sizeof(kek_symbol_t) + length));
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

/******************************************************************************/
/* old space */

os_remember_set_t *gc_os_remember_set_g;
os_item_t *gc_os_items_g;

void gc_os_init() {
	gc_os_remember_set_g = NULL;
	gc_os_items_g = NULL;
}

void gc_os_free() {
	os_remember_set_t *rsptr;
	os_remember_set_t *rstmpptr;

	os_item_t *itemptr;
	os_item_t *itemtmpptr;

	for (rsptr = gc_os_remember_set_g; rsptr;
			rstmpptr = rsptr, rsptr = rsptr->next, free(rstmpptr))
		;

	for (itemptr = gc_os_items_g; itemptr; itemtmpptr = itemptr, itemptr =
			itemptr->next, free(itemtmpptr->obj), free(itemtmpptr))
		;
}

/* ONEDIT: this function must somehow correspond with gc_cheney_copy_neighbor */
void gc_os_rec_cpy_neighbors(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;

	assert(IS_PTR(obj));
	assert(OBJ_TYPE_CHECK(obj));

	if (obj->h.t == KEK_COPIED) {
		vm_debug(DBG_OLD, "gc_os_rec_cpy_neighbors: it's a copy, skip\n");
		*objptr = (kek_obj_t *) obj->h.cls;
		return;
	}

	if (!gc_os_is_in_old(obj)) {
		assert(gc_cheney_ptr_in_from_space(obj, vm_obj_size(obj)));
	}

	gc_os_add_item(objptr);

	switch (obj->h.t) {
	case KEK_NIL:
		assert(0 && "only in cost tbl");
		break;
	case KEK_INT:
		break;
	case KEK_STR:
		break;
	case KEK_SYM:
		break;
	case KEK_ARR: {
		kek_array_objs_t *arr_objs;
		int i;

		vm_debug(DBG_OLD, "KEK_ARR obj=%p obj->k_arr.elems=%p\n", //
				obj, obj->k_arr.elems);

		vm_assert(obj->k_arr.elems != NULL,
				"arr=%p is copied into the " "old space, but its elems are NULL\n",
				obj);

		arr_objs = KEK_ARR_OBJS(obj);
		assert(arr_objs->h.h.t == KEK_ARR_OBJS);

		/* this will copy the structure with the pointers to the objs */
		gc_os_add_item((kek_obj_t **) &arr_objs);

		/* update elems ptr */
		obj->k_arr.elems = &(arr_objs->elems[0]);

		/* now recursive copy the elems */
		for (i = 0; i < obj->k_arr_objs.h.length; i++) {
			if (IS_PTR(obj->k_arr_objs.elems[i])) {
				gc_os_rec_cpy_neighbors(&obj->k_arr_objs.elems[i]);
			}
		}

		break;
	}
	case KEK_ARR_OBJS:
		vm_error("gc_os_rec_cpy_neighbors should NOT copy KEK_ARR_OBJS. "
				"This should be done when copying KEK_ARR.");
		break;
	case KEK_EXINFO:
		assert(0 && "only in const tbl");
		break;
	case KEK_EXPT: {
		/* copy union _kek_obj * msg; */
		kek_obj_t * msg = obj->k_expt.msg;
		if (msg != NULL && IS_PTR(msg)) {
			gc_os_add_item(&(obj->k_expt.msg));
		}
		break;
	}
	case KEK_FILE:
		break;
	case KEK_TERM:
		// assert(0 && "this should  be in oldspace");
		/* todo: bude v oldspace (vsechno  co je v sys) */
		break;
	case KEK_UDO: {
		int i;
		bool verbose = false;
		int total_size = obj->h.cls->total_syms_instance_cnt
				+ obj->h.cls->syms_instance_offset;
		if (!strcmp(obj->h.cls->name, "Reader")) {
			cp_reader: //
			verbose = true;
		}
		for (i = 0; i < total_size; i++) {
			kek_obj_t * var = obj->k_udo.inst_var[i];
			if (var != NULL && IS_PTR(var)) {
				if (verbose) {
					vm_debug(DBG_BC,
							"Copying inst_var[%d], type = %d, ptr = %p, ", i,
							var->h.t, var);
				}

				gc_os_rec_cpy_neighbors(&(obj->k_udo.inst_var[i]));

				if (verbose) {
					vm_debug(DBG_BC, "new_ptr = %p.\n", obj->k_udo.inst_var[i]);
				}
			}
		}
	}
		break;
	case KEK_CLASS:
		assert(0);
		break;
	case KEK_COPIED:
		vm_error("gc_os_rec_cpy_neighbors got KEK_COPIED\n");
		break;
	default:
		vm_error("Unknown obj->h.t=%d in gc_cheney_copy_inner_objs\n",
				obj->h.t);
		break;
	}
}

/**
 * FIXME: we don't use ALIGNED here
 */
void gc_os_add_item(kek_obj_t **objptr) {
	kek_obj_t *obj = *objptr;
	os_item_t *os_item;

	assert(IS_PTR(obj));
	assert(OBJ_TYPE_CHECK(obj));
	assert(gc_cheney_ptr_in_from_space(obj, vm_obj_size(obj)));

	os_item = malloc(sizeof(os_item_t));
	assert(os_item);

	os_item->next = NULL;
	os_item->obj = malloc(vm_obj_size(obj));
	assert(os_item->obj);

	/* the obj is a pointer in from space */
	(void) memcpy(os_item->obj, obj, vm_obj_size(obj));

	obj->h.state = OBJ_OLD_WHITE;

	/* add the os_item to the global LL */
	if (gc_os_items_g == NULL) {
		gc_os_items_g = os_item;
	} else {
		assert(gc_os_items_g->next == NULL);
		gc_os_items_g->next = os_item;
	}

	obj->h.t = KEK_COPIED;
	obj->h.cls = (struct _class *) os_item->obj;

	*objptr = os_item->obj;
}

bool gc_os_is_in_old(kek_obj_t *obj) {
	vm_assert(obj != NULL, "obj is null (%d)\n", 0);

	switch (obj->h.state) {
	case OBJ_OLD_WHITE:
	case OBJ_OLD_GRAY:
	case OBJ_OLD_BLACK:
		return (true);
	case OBJ_2ND_GEN_YOUNG:
	case OBJ_1ST_GEN_YOUNG:
		return (false);
	default:
		vm_error("gc_os_is_in_old: invalid obj->h.state %d, obj->h.t=%d\n",
				obj->h.state, obj->h.t);
		return (false);
	}
}

bool gc_os_is_in_new(kek_obj_t *obj) {
	vm_assert(obj != NULL, "obj is null (%d)\n", 0);

	switch (obj->h.state) {
	case OBJ_OLD_WHITE:
	case OBJ_OLD_GRAY:
	case OBJ_OLD_BLACK:
		return (false);
	case OBJ_2ND_GEN_YOUNG:
	case OBJ_1ST_GEN_YOUNG:
		return (true);
	default:
		vm_error("gc_os_is_in_new: invalid state %d\n", obj->h.state);
		return (false);
	}
}

void gc_os_write_barrier(kek_obj_t **dst_objptr, kek_obj_t **objptr) {
	kek_obj_t *dst_obj = *dst_objptr;
	kek_obj_t *obj = *objptr;

	os_remember_set_t *rs_on; /* remember set: old->new */
//	os_remember_set_t *rs_bw; /* remember set: black->white */

	/* this happens when dst_obj is not in the old space neither in
	 * the new space */
	if (dst_obj == NULL) {
		return;
	}

	/* old -> new */
	if (gc_os_is_in_old(dst_obj) && gc_os_is_in_new(obj)) {
		rs_on = malloc(sizeof(os_remember_set_t));
		assert(rs_on);

		rs_on->next = NULL;
		rs_on->old_obj = dst_obj;
		rs_on->new_obj = objptr;

		if (gc_os_remember_set_g == NULL) {
			gc_os_remember_set_g = rs_on;
		} else {
			assert(gc_os_remember_set_g->next == NULL);
			gc_os_remember_set_g->next = rs_on;
		}
	}

	/* black -> white */
	/* TODO */
}

/******************************************************************************/
