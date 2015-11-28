/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/******************************************************************************/
/* objects ********************************************************************/

struct _class;
union _kek_obj;

typedef enum _type {
	KEK_NIL, KEK_INT, KEK_STR, KEK_SYM, KEK_ARR, KEK_EXINFO, KEK_EXPT, KEK_FILE, KEK_TERM, KEK_UDO, KEK_CLASS
} type_t;

typedef struct _header {
	type_t t;
	struct _class * cls; /* Each object needs a pointer to its class. */

	/* FIXME TODO compiler will broke? will i need it anyway? */
	/* uint32_t uid; */ /* gc wants to know */
} header_t;

/* extern uint32_t uid_g; */ /* which uid was the last one */

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
	// FIXME: We should mark final classes that cannot be parents.
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
	// FIXME: We should mark final objects that cannot be parents.
	int length;
	int alloc_size;
	/* Loader will need to transform each constant_array_t to this format */
	union _kek_obj ** elems;
} kek_array_t;

typedef struct _try_range {
	int try_addr;
	int catch_addr;
} try_range_t;

typedef struct _kek_exinfo {
	header_t h;
	union _kek_obj * obj_thrown;
#if !defined(__LP64__)
	uint32_t padding;
#endif
	int length;
	try_range_t blocks[1];
} kek_exinfo_t;

typedef struct _kek_except {
	header_t h;
	// We imitate the kek_udo_t layout so that derived objects can exist.
	int var_offset;
	union _kek_obj * msg;
} kek_except_t;

typedef struct _kek_file {
	header_t h;
	// We imitate the kek_udo_t layout so that derived objects can exist.
	int var_offset;
	FILE * f_handle;
} kek_file_t;

typedef struct _kek_term {
	header_t h;
} kek_term_t;

/* user-defined object */
typedef struct _kek_udo {
	header_t h;
	// When the parent object is builtin we need to reserve space for its vars.
	// Compile-time offsets to inst_var will be incremented by var_offset
	// using INST_VAR macro.
	int var_offset;
	union _kek_obj * inst_var[1]; /* inst_var[syms_instance_cnt] */
} kek_udo_t;

typedef union _kek_obj {
	header_t h;
	kek_nil_t k_nil;
	kek_int_t k_int;
	kek_string_t k_str;
	kek_symbol_t k_sym;
	kek_array_t k_arr;
	kek_exinfo_t k_exi;
	kek_except_t k_expt;
	kek_file_t k_fil;
	kek_term_t k_term;
	kek_udo_t k_udo;
} kek_obj_t;

#if defined(__LP64__)

#define IS_PTR(obj) (((uint64_t)(obj) & 3) == 0)

#define IS_CHAR(obj) (((uint64_t)(obj) & 3) == 2)
#define MAKE_CHAR(c) (((uint64_t)(c) << 2) | 2)
#define CHAR_VAL(c) (char)((uint64_t)(c) >> 2)

#define IS_INT(obj) (((uint64_t)(obj) & 1) || (IS_PTR(obj) && ((obj)->h.t == KEK_INT)))
#define INT_VAL(obj) (((uint64_t)(obj) & 1) ? \
		((int32_t)(int64_t)(obj) >> 1) : ((obj)->k_int.value))

#define IS_DPTR(obj) (((uint64_t)(obj) & 3) == 3)
#define MAKE_DPTR(obj) ((uint64_t)(obj) | 3)
#define DPTR_VAL(obj) (*(kek_obj_t**)((uint64_t)(obj) & ~3ULL))

#else

#define IS_PTR(obj) (((uint32_t)(obj) & 3) == 0)

#define IS_CHAR(obj) (((uint32_t)(obj) & 3) == 2)
#define MAKE_CHAR(c) (((uint32_t)(c) << 2) | 2)
#define CHAR_VAL(c) (char)((uint32_t)(c) >> 2)

#define IS_INT(obj) (((uint32_t)(obj) & 1) || (IS_PTR(obj) && ((obj)->h.t == KEK_INT)))
#define INT_VAL(obj) (((uint32_t)(obj) & 1) ? \
		((int32_t)(obj) >> 1) : ((obj)->k_int.value))

#define IS_DPTR(obj) (((uint32_t)(obj) & 3) == 3)
#define MAKE_DPTR(obj) ((uint32_t)(obj) | 3)
#define DPTR_VAL(obj) (*(kek_obj_t**)((uint32_t)(obj) & ~3))

#endif

#define IS_NIL(obj) (IS_PTR(obj) && (obj)->h.t == KEK_NIL)
#define IS_STR(obj) (IS_PTR(obj) && (obj)->h.t == KEK_STR)
#define IS_SYM(obj) ((obj)->h.t == KEK_SYM)
#define IS_ARR(obj) (IS_PTR(obj) && (obj)->h.t == KEK_ARR)
#define IS_UDO(obj) (IS_PTR(obj) && (obj)->h.t == KEK_UDO)
#define IS_CLASS(obj) (IS_PTR(obj) && (obj)->h.t == KEK_CLASS)

#define NIL CONST(0)

#define INST_VAR(obj, idx) ((obj)->k_udo.inst_var[(idx) + (obj)->k_udo.var_offset])

#endif /* TYPES_H_ */
