/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "vm.h"

#define ARR_INIT_SIZE 512

kek_obj_t * alloc_array(class_t * arr_class);
kek_obj_t ** alloc_arr_elems(size_t size);

kek_obj_t * alloc_string(class_t * str_class, size_t length);

#endif