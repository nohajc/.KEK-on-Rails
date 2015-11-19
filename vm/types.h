/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>
#include <stdio.h>

/******************************************************************************/
/* objects ********************************************************************/

struct _class;
union _kek_obj;

typedef enum _type {
	KEK_NIL, KEK_INT, KEK_STR, KEK_SYM, KEK_ARR, KEK_FILE, KEK_UDO, KEK_CLASS
} type_t;

typedef struct _header {
	type_t t;
	struct _class * cls; /* Each object needs a pointer to its class. */
	size_t size; /* total size of the object */
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
	int alloc_size;
	/* Loader will need to transform each constant_array_t to this format */
	union _kek_obj ** elems;
} kek_array_t;

typedef struct _kek_file {
	header_t h;
	FILE * f_handle;
} kek_file_t;

/* user-defined object */
typedef struct _kek_udo {
	header_t h;
	union _kek_obj * inst_var[1]; /* inst_var[syms_instance_cnt] */
} kek_udo_t;

typedef union _kek_obj {
	header_t h;
	kek_nil_t k_nil;
	kek_int_t k_int;
	kek_string_t k_str;
	kek_symbol_t k_sym;
	kek_array_t k_arr;
	kek_file_t k_fil;
	kek_udo_t k_udo;
} kek_obj_t;

#if defined(__LP64__)
#define IS_PTR(obj) (((uint64_t)(obj) & 3) == 0)
#define IS_CHAR(obj) (((uint64_t)(obj) & 3) == 2)
#define MAKE_CHAR(c) (((uint64_t)(c) << 2) | 2)
#define CHAR_VAL(c) (char)((uint64_t)(c) >> 2)
#else
#define IS_PTR(obj) (((uint32_t)(obj) & 3) == 0)
#define IS_CHAR(obj) (((uint32_t)(obj) & 3) == 2)
#define MAKE_CHAR(c) (((uint32_t)(c) << 2) | 2)
#define CHAR_VAL(c) (char)((uint32_t)(c) >> 2)
#endif

#define IS_NIL(obj) ((obj)->h.t == KEK_NIL)
#define IS_INT(obj) ((obj)->h.t == KEK_INT)
#define IS_STR(obj) ((obj)->h.t == KEK_STR)
#define IS_SYM(obj) ((obj)->h.t == KEK_SYM)
#define IS_ARR(obj) ((obj)->h.t == KEK_ARR)
#define IS_UDO(obj) ((obj)->h.t == KEK_UDO)
#define IS_CLASS(obj) ((obj)->h.t == KEK_CLASS)

#define INT_VAL(obj) (obj)->k_int.value

#define NIL CONST(0)



#endif /* TYPES_H_ */
