/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "vm.h"
#include "loader.h"


/******************************************************************************/
/* global variables */

//class_t *classes_g;

/******************************************************************************/
/* debugging/printing code */

void vm_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}

#if DEBUG
void vm_debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}
#else
void vm_debug(const char *format, ...) {
}
#endif
/******************************************************************************/

void vm_init_builtin_classes(void) {
	init_kek_array_class();
	// TODO: String, ...
}