/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef MEMORY_H_
#define MEMORY_H_

#include "vm.h"

#define ARR_INIT_SIZE 512

struct _class;

// Integers are primitive objects without methods - they don't need a class pointer
union _kek_obj * alloc_integer(void);
union _kek_obj * alloc_array(struct _class * arr_class);
void alloc_arr_elems(struct _kek_array * arr);
void realloc_arr_elems(struct _kek_array * arr, int length);
union _kek_obj * alloc_string(struct _class * str_class, int length);
union _kek_obj * alloc_udo(struct _class * arr_class);

#endif
