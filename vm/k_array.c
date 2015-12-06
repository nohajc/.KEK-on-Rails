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
	char name[] = "Array";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 2;
	// TODO: add native methods such as length(), get_idx(), get_idxa()
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
}

// Constructor of an empty array.
// Can be called from bytecode, so we use our custom stack.
void new_array(void) {
	kek_array_t * arr = (kek_array_t*) THIS;
	native_new_array(arr);

	BC_RET_SELF;
}

void native_new_array(kek_array_t * arr) {
	arr->length = 0;
	alloc_arr_elems(arr);
}

void native_arr_elem_set(kek_array_t * arr, int idx, kek_obj_t * obj) {
	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);

	if (idx >= arr->alloc_size) {
		native_grow_array(arr, idx + 1);
		// TODO: this is problematic
		// We need to update arr pointer if it was copied by GC
	} else if (idx >= arr->length) {
		arr->length = idx + 1;
	}
	arr->elems[idx] = obj;

	gc_rootset_remove(id);
}

void array_length(void) {
	kek_array_t * arr = (kek_array_t*) THIS;
	kek_obj_t * kek_len = native_array_length(arr);

	PUSH(kek_len);
	BC_RET;
}

void array_append(void) {
	kek_array_t * arr = (kek_array_t*) THIS;
	uint32_t id = gc_rootset_add((kek_obj_t **)&arr);
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
		arr = (kek_array_t*)THIS;
		obj = ARG(0);
	}
	else {
		arr->length = new_len;

	}

	for (i = old_len, j = 0; j < obj->k_arr.length; ++i, ++j) {
		arr->elems[i] = obj->k_arr.elems[j];
	}
	PUSH(NIL);
	BC_RET;
	gc_rootset_remove(id);
}

kek_obj_t * native_array_length(kek_array_t * arr) {
	kek_obj_t * kek_len = (kek_obj_t*) make_integer(arr->length);

	return kek_len;
}

void native_grow_array(kek_array_t * arr, int length) {
	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);
	kek_array_objs_t *arr_objs;
	int i;
	realloc_arr_elems(arr, length);

	// TODO: we need arr pointer update
	// if realloc triggered GC
//	for (i = arr->length; i < length - 1; ++i) {
//		arr->elems[i] = NIL;
//	}

	arr_objs = (kek_array_objs_t *) ((uint8_t *) arr->elems
			- sizeof(kek_array_objs_header_t));
	assert(arr_objs->h.h.t == KEK_ARR_OBJS);

	arr_objs->h.length = length;
	arr->length = length;

	gc_rootset_remove(id);
}
