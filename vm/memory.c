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

kek_obj_t ** alloc_arr_elems(size_t size) {
	kek_obj_t **ret = malloc(size * sizeof(kek_obj_t*));
	assert(ret);

	return ret;
}

kek_obj_t * alloc_string(class_t * str_class, size_t length) {
	kek_obj_t * ret = malloc(sizeof(kek_string_t) + length);
	assert(ret);
	ret->h.t = KEK_STR;
	ret->h.cls = str_class;

	return ret;
}
