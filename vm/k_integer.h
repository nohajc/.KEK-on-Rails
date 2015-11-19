/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_INTEGER_H_
#define K_INTEGER_H_

#include "types.h"

void new_integer(void); // Constructor only
void native_new_integer(kek_int_t * n, int value); // Constructor only
kek_int_t * make_integer(int value); // Allocation and constructor in one

#endif