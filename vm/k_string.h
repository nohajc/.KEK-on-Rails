/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_STRING_H_
#define K_STRING_H_

#include "vm.h"
#include "memory.h"

void init_kek_string_class(void);
union _kek_obj * new_string_from_cstring(const char *);

#endif
