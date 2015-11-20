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
	classes_g[classes_cnt_g].methods_cnt = 1;
	// TODO: add native methods such as length(), get_idx(), get_idxa()
	classes_g[classes_cnt_g].methods = malloc(
		classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "length", 0, false, array_length);

	classes_g[classes_cnt_g].allocator = alloc_array;

	classes_g[classes_cnt_g].constructor = malloc(sizeof(method_t));
	vm_init_native_method(classes_g[classes_cnt_g].constructor, "Array", 0, false, new_array);

	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

// Constructor of an empty array.
// Can be called from bytecode, so we use our custom stack.
void new_array(void) {
	kek_array_t * arr = (kek_array_t*)THIS;
	native_new_array(arr);

	BC_RET_SELF;
}

void native_new_array(kek_array_t * arr) {
	arr->length = 0;
	alloc_arr_elems(arr);
}

void native_arr_elem_set(kek_array_t * arr, int idx, kek_obj_t * obj) {
	if (idx >= arr->alloc_size) {
		native_grow_array(arr, idx + 1);
	}
	else if (idx >= arr->length) {
		arr->length = idx + 1;
	}
	arr->elems[idx] = obj;
}

void array_length(void) {
	kek_array_t * arr = (kek_array_t*)THIS;
	kek_obj_t * kek_len = native_array_length(arr);

	PUSH(kek_len);
	BC_RET;
}

// TODO: We could cache the kek_int object containing length
// so we don't have to create a new one on each call.
kek_obj_t * native_array_length(kek_array_t * arr) {
	kek_obj_t * kek_len = (kek_obj_t*)make_integer(arr->length);

	return kek_len;
}

void native_grow_array(kek_array_t * arr, int length) {
	int i;
	realloc_arr_elems(arr, length);

	for (i = arr->length; i < length - 1; ++i) {
		arr->elems[i] = NIL;
	}
	arr->length = length;
}
