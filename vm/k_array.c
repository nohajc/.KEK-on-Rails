/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "vm.h"
#include "memory.h"
#include "k_array.h"
#include "k_integer.h"
#include "stack.h"

void init_kek_array_class(void) {

	vm_debug(DBG_GC,
			"init_kek_array_class BEGIN +++++++++++++++++++++++++++\n");

	char name[] = "Array";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 2;
	classes_g[classes_cnt_g].methods = malloc(
			classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "length", 0,
	false, array_length);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[1], "append", 1,
	false, array_append);

	classes_g[classes_cnt_g].allocator = alloc_array;

	classes_g[classes_cnt_g].constructor = malloc(sizeof(method_t));
	vm_init_native_method(classes_g[classes_cnt_g].constructor, "Array", 0,
	false, new_array);

	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;
	classes_cnt_g++;
	vm_debug(DBG_GC, "init_kek_array_class END ----------------------------\n");
}

void arr_set_length(kek_array_t *arr, int length) {
	arr->length = length;
//	assert(arr != NULL);

	vm_assert(KEK_ARR_OBJS(arr)->h.h.t == KEK_ARR_OBJS,
			"arr=%p, arr_elems=%p, arr_objs=%p, arr_objs.type=%d\n", //
			(void* ) arr, (void* ) arr->elems, KEK_ARR_OBJS(arr),
			KEK_ARR_OBJS(arr)->h.h.t);

	KEK_ARR_OBJS(arr)->h.length = length;
}

void arr_set_alloc_size(kek_array_t *arr, int size) {
	arr->alloc_size = size;
	KEK_ARR_OBJS(arr)->h.alloc_size = size;
}

// Constructor of an empty array.
// Can be called from bytecode, so we use our custom stack.
void new_array(void) {
	vm_debug(DBG_GC, "new array BEGIN +++++++++++++++++++++++++++++++++++++\n");

	kek_array_t * arr = (kek_array_t*) THIS;

	size_t arr_init_size_real = sizeof(kek_array_objs_t)
			+ (ARR_INIT_SIZE - 1) * sizeof(kek_obj_t *);

//	arr_init_size_real = ALIGNED(arr_init_size_real);

	/* FIXME? */
	if (gc_type_g != GC_NONE && !gc_cheney_can_malloc(arr_init_size_real)) {
		vm_debug(DBG_GC, "new_array: not enought space, force gc! arr.as=%d\n",
				arr->alloc_size);

		gc_cheney_scavenge();

		if (!gc_cheney_can_malloc(arr_init_size_real)) {
			vm_error("not enough space even after gc.\n");
		}
	}

	/* gc could changed this ptr. */
	arr = (kek_array_t*) THIS;

	native_new_array(arr);

	BC_RET_SELF
	; /* sorry, I don't know how to format this */
	vm_debug(DBG_GC, "new array END ---------------------------------------\n");
}

void native_new_array(kek_array_t * arr) {
	vm_debug(DBG_GC, "native_new_array BEGIN ++++++++++++++++++++++++++++++\n");
	arr->length = 0;
	//arr->alloc_size = ARR_INIT_SIZE;
	arr->elems = alloc_arr_elems(ARR_INIT_SIZE, 0);
	arr_set_alloc_size(arr, ARR_INIT_SIZE);
	vm_debug(DBG_GC, "native_new_array END --------------------------------\n");
}

void native_arr_elem_set(kek_array_t * arr, int idx, kek_obj_t * obj) {
	vm_debug(DBG_GC, "native_arr_elem_set BEGIN +++++++++++++++++++++++++++\n");

	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);

	if (idx >= arr->alloc_size) {
		native_grow_array(arr, idx + 1);
	} else if (idx >= arr->length) {
		arr_set_length(arr, idx + 1);
	}
	arr->elems[idx] = obj;

	gc_rootset_remove_id(id);

	vm_debug(DBG_GC, "native_arr_elem_set END -----------------------------\n");
}

void array_length(void) {
	vm_debug(DBG_GC, "array_length BEGIN ++++++++++++++++++++++++++++++++++\n");
	kek_array_t * arr = (kek_array_t*) THIS;
	kek_obj_t * kek_len = native_array_length(arr);

	PUSH(kek_len);
	BC_RET
	; /* sorry, I don't know how to format this */

	vm_debug(DBG_GC, "array_length END ------------------------------------\n");
}

void array_append(void) {
	vm_debug(DBG_GC, "array_append BEGIN ++++++++++++++++++++++++++++++++++\n");
	kek_array_t * arr = (kek_array_t*) THIS;
	//uint32_t id = gc_rootset_add((kek_obj_t **) &arr);
	kek_obj_t * obj = ARG(0);
	int new_len, old_len;
	int i, j;

	if (!IS_ARR(obj)) {
		vm_error("Expected array as argument.\n");
	}

	new_len = arr->length + obj->k_arr.length;
	old_len = arr->length;
	if (new_len > arr->alloc_size) {
		native_grow_array(arr, new_len);
		// GC phase could have occured, we need to update pointers
		arr = (kek_array_t*) THIS;
		obj = ARG(0);
	} else {
		arr->length = new_len;

	}

	for (i = old_len, j = 0; j < obj->k_arr.length; ++i, ++j) {
		arr->elems[i] = obj->k_arr.elems[j];
	}
	PUSH(NIL);
	BC_RET
	; /* sorry, I don't know how to format this */
	//gc_rootset_remove(id);
	vm_debug(DBG_GC, "array_append END ------------------------------------\n");
}

kek_obj_t * native_array_length(kek_array_t * arr) {
	kek_obj_t * kek_len = (kek_obj_t*) make_integer(arr->length);

	assert(arr->length == KEK_ARR_OBJS(arr)->h.length);

	return kek_len;
}

void native_grow_array(kek_array_t * arr, int length) {
	vm_debug(DBG_GC, "array_grow_array BEGIN ++++++++++++++++++++++++++++++\n");

	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);
	kek_array_objs_t *arr_objs;
	int i;
	arr_realloc_elems(arr, length);

//	for (i = arr->length; i < length - 1; ++i) {
//		arr->elems[i] = NIL;
//	}

	arr_objs = (kek_array_objs_t *) ((uint8_t *) arr->elems
			- sizeof(kek_array_objs_header_t));
	assert(arr_objs->h.h.t == KEK_ARR_OBJS);

	arr_set_length(arr, length);

	gc_rootset_remove_id(id);
	vm_debug(DBG_GC, "array_grow_array END --------------------------------\n");
}
