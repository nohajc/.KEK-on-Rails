/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "types.h"

#define ARR_INIT_SIZE 512

typedef kek_obj_t *(*alloc_ptr)(void);

kek_obj_t * alloc_array(void);
kek_obj_t ** alloc_arr_elems(size_t size);

#endif