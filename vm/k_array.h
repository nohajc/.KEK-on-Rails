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
void native_set(kek_array_t * arr, int idx, kek_obj_t * obj);
kek_obj_t * native_get(kek_array_t * arr, int idx);

#endif