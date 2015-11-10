/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>

/******************************************************************************/
/* objects ********************************************************************/

union _kek_obj;

typedef enum _type {
	KEK_NIL, KEK_INT, KEK_STR, KEK_SYM, KEK_ARR
} type_t;

typedef struct _header {
	type_t t;
} header_t;

/* nil - immutable, singleton */
typedef struct _kek_nil {
	header_t h;
} kek_nil_t;

/* integer - immutable */
typedef struct _kek_int {
	header_t h;
	int value;
} kek_int_t;

/* string - immutable */
typedef struct _kek_string {
	header_t h;
	int length;
	char string[1];
} kek_string_t;

/* symbol - immutable */
typedef struct _kek_symbol {
	header_t h;
	int length;
	char symbol[1];
} kek_symbol_t;

/* array - mutable */
typedef struct _kek_array {
	header_t h;
	int length;
	/* Loader will need to transform each constant_array_t to this format */
	union _kek_obj ** elems;
} kek_array_t;

typedef union _kek_obj {
	kek_nil_t k_nil;
	kek_int_t k_int;
	kek_string_t k_str;
	kek_symbol_t k_sym;
	kek_array_t k_arr;
} kek_obj_t;

#endif /* TYPES_H_ */