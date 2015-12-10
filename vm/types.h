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

/******************************************************************************/
/* object type */

typedef enum _type {
	KEK_NIL, //
	KEK_INT, //
	KEK_STR, //
	KEK_SYM, //
	KEK_ARR, //4
	KEK_ARR_OBJS, //
	KEK_EXINFO, //
	KEK_EXPT, //
	KEK_FILE, //
	KEK_TERM, //
	KEK_UDO, //
	KEK_CLASS, //
	KEK_STACK, //
	KEK_COPIED // when gc copies and obj, this will be the type of the old one
} type_t;

#define TYPE_CHECK(type) (((type) >= 0) && ((type) <= 10))
#define OBJ_TYPE_CHECK(obj) (TYPE_CHECK((obj)->h.t))

static const char *type_str_g[] = { "NIL", "INT", "STR", "SYM", "ARR", "EXINFO",
		"EXPT", "FILE", "TERM", "UDO", "CLASS", "STACK", "COPIED" };

/******************************************************************************/

typedef struct _header {
	type_t t;

	/* Each object needs a pointer to its class. */
	/* This pointer may use GC for forwarding address */
	struct _class * cls;

	/* todo: pocet preziti */
	/* gc se bude moc volat nasilne, gc bude mit zamek, kdy se gc volat nebude*/
	/* todo: gc se bude volat s rezervou */
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
typedef struct _kek_array_objs_header {
	header_t h;
	int length;
} kek_array_objs_header_t;

typedef struct _kek_array_objs {
	kek_array_objs_header_t h;
	union _kek_obj *elems[1];
} kek_array_objs_t;

typedef struct _kek_array {
	header_t h;
	// FIXME: We should mark final objects that cannot be parents.
	int length;
	int alloc_size;
	/* Loader will need to transform each constant_array_t to this format */
	union _kek_obj **elems;
} kek_array_t;

#define KEK_ARR_OBJS(obj) ((kek_array_objs_t *) ((uint8_t *) \
		((kek_array_t *)(obj))->elems \
		- sizeof(kek_array_objs_header_t)))

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

/******************************************************************************/

/******************************************************************************/

typedef union _kek_obj {
	header_t h;
	kek_nil_t k_nil;
	kek_int_t k_int;
	kek_string_t k_str;
	kek_symbol_t k_sym;
	kek_array_t k_arr;
	kek_array_objs_t k_arr_objs;
	kek_exinfo_t k_exi;
	kek_except_t k_expt;
	kek_file_t k_fil;
	kek_term_t k_term;
	kek_udo_t k_udo;
} kek_obj_t;

#if defined(__LP64__)
typedef uint64_t ptruint_t;
#else
typedef uint32_t ptruint_t;
#endif

#define IS_PTR(obj) (((ptruint_t)(obj) & 3) == 0)

#define IS_CHAR(obj) (((ptruint_t)(obj) & 3) == 2)
#define MAKE_CHAR(c) (((ptruint_t)(c) << 2) | 2)
#define CHAR_VAL(c) (char)((ptruint_t)(c) >> 2)

#define IS_INT(obj) \
		(((ptruint_t)(obj) & 1) || (IS_PTR(obj) && ((obj)->h.t == KEK_INT)))
#define INT_VAL(obj) (((ptruint_t)(obj) & 1) ? \
		(int32_t)((ptruint_t)(obj) >> 1) : ((obj)->k_int.value))

#define IS_DPTR(obj) (((ptruint_t)(obj) & 3) == 3)
#define MAKE_DPTR(obj, addr) (((ptruint_t)(addr) - (ptruint_t)(obj)) | 3)
#define DPTR_VAL(obj, addr) \
		((kek_obj_t**)((uint8_t*)(obj) + ((ptruint_t)(addr) & ~3ULL)))

#define IS_NIL(obj) (IS_PTR(obj) && (obj)->h.t == KEK_NIL)
#define IS_STR(obj) (IS_PTR(obj) && (obj)->h.t == KEK_STR)
#define IS_SYM(obj) ((obj)->h.t == KEK_SYM)
#define IS_ARR(obj) (IS_PTR(obj) && (obj)->h.t == KEK_ARR)
#define IS_UDO(obj) (IS_PTR(obj) && (obj)->h.t == KEK_UDO)
#define IS_CLASS(obj) (IS_PTR(obj) && (obj)->h.t == KEK_CLASS)

#define NIL CONST(0)

#define INST_VAR(obj, idx) ((obj)->k_udo.inst_var[(idx) + (obj)->k_udo.var_offset])

#endif /* TYPES_H_ */
