/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_SYS_H_
#define K_SYS_H_

#include "types.h"

void init_kek_sys_class(void);
void sys_rand(void);
void sys_srand(void);
void sys_time(void);

#endif