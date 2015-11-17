/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_FILE_H_
#define K_FILE_H_

#include "types.h"

void init_kek_file_class(void);
void new_file(void);
void file_close(void);
void file_readln(void);
void file_writeln(void);

#endif