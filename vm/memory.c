/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <assert.h>
#include "memory.h"

kek_obj_t * alloc_array(class_t * arr_class) {
	kek_obj_t * ret = malloc(sizeof(kek_array_t));
	assert(ret);

	ret->h.t = KEK_ARR;
	ret->h.cls = arr_class;

	return ret;
}

void alloc_arr_elems(kek_array_t * arr) {
	arr->elems = malloc(ARR_INIT_SIZE * sizeof(kek_obj_t*));
	assert(arr->elems);
	arr->alloc_size = ARR_INIT_SIZE;
}

void realloc_arr_elems(struct _kek_array * arr, int length) {
	while (arr->alloc_size < length) {
		arr->alloc_size *= 2;
	}

	arr->elems = realloc(arr->elems, arr->alloc_size);
}

kek_obj_t * alloc_string(class_t * str_class, int length) {
	kek_obj_t * ret = malloc(sizeof(kek_string_t) + length);
	assert(ret);
	ret->h.t = KEK_STR;
	ret->h.cls = str_class;

	return ret;
}

kek_obj_t * alloc_integer(void) {
	kek_obj_t * ret = malloc(sizeof(kek_int_t));
	assert(ret);

	ret->h.t = KEK_INT;
	ret->h.cls = NULL;

	return ret;
}

kek_obj_t * alloc_udo(class_t * udo_class) {
	// Calloc is used to avoid valgrind warnings about
	// invalid reads/writes to uninitialized memory
	kek_obj_t * ret = calloc(udo_class->syms_instance_cnt - 1, sizeof(kek_udo_t));
	assert(ret);

	ret->h.t = KEK_UDO;
	ret->h.cls = udo_class;

	return ret;
}

kek_obj_t * alloc_file(class_t * file_class) {
	kek_obj_t * ret = malloc(sizeof(kek_file_t));
	assert(ret);

	ret->h.t = KEK_FILE;
	ret->h.cls = file_class;

	return ret;
}