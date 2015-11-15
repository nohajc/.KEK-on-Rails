/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_ARRAY_H_
#define K_ARRAY_H_

#include "types.h"

void init_kek_array_class(void);
void new_array(void);
void native_new_array(kek_array_t * arr);
void native_arr_elem_set(kek_array_t * arr, int idx, kek_obj_t * obj);
kek_obj_t * native_arr_elem_get(kek_array_t * arr, int idx);
void array_length(void);
kek_obj_t * native_array_length(kek_array_t * arr);
void native_grow_array(kek_array_t * arr, int length);

#endif