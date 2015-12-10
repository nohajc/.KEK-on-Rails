/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef K_STRING_H_
#define K_STRING_H_

#include "types.h"

void init_kek_string_class(void);
union _kek_obj * new_string_from_cstring(const char *);
union _kek_obj * new_string_from_concat(const char *, const char *);

// Instance methods
void string_length(void);
void string_split(void);
void string_toInt(void);
void string_replace(void);

// Static methods
void string_fromInt(void);
void string_fromArray(void);

#endif
