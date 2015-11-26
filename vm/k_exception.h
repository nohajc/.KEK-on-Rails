/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_EXCEPTION_H_
#define K_EXCEPTION_H_

#include "types.h"

void init_kek_exception_class(void);
void new_exception(void);
void native_new_exception(kek_except_t * e, kek_obj_t * msg);
kek_except_t * make_exception(kek_obj_t * msg);
void exception_msg(void);
void exception_type(void);

#endif