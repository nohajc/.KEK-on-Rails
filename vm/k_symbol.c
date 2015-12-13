/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "k_symbol.h"
#include "vm.h"
#include "memory.h"

kek_obj_t * new_symbol_from_cstring(const char * cstr){
	size_t len = strlen(cstr);
	kek_symbol_t * ksym = (kek_symbol_t *) alloc_symbol(len);
	ksym->length = len;
	strcpy(ksym->symbol, cstr);

	return ((kek_obj_t *) ksym);
}