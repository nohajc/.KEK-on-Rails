/*
 * bcout.h
 *
 *  Created on: Nov 3, 2015
 *      Author: n
 */

#ifndef COMPILER_BCOUT_H_
#define COMPILER_BCOUT_H_

#define DEBUG 1
#define DEFAULT_BUFFER_SIZE 8192

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "tabsym.h"

enum Operator {
	Plus,
	Minus,
	Times,
	Divide,
	Modulo,
	Eq,
	NotEq,
	Less,
	Greater,
	LessOrEq,
	GreaterOrEq,
	LogOr,
	LogAnd,
	BitOr,
	BitAnd,
	Xor,
	Lsh,
	Rsh,
	Error
};

/*
 Extra instruction info:
 NIL - no special meaning
 BRK - break
 */
typedef enum _label {
	NIL, /**/
	BRK
} label_t;

typedef enum _bc {
	NOP, /**/
	BOP, /**/
	UNM, /**/
	DR, /* dereference */
	ST, /* store */
	IFNJ, /* if zero, jump */
	JU, /* jump */
	WRT, /* write */
	RD, /* read */
	DUP, /* duplicate the top of the stack */
	SWAP, /* swap the first two items on the stack */
	NOT, /* not */
	STOP, /* terminate */

	RET, /* */

	CALL, /* arg: index to const table */
	CALLS, /* arg: index to const table, calls method of the superclass */
	CALLE, /* call external. arg: index to const table,
	 start search at the given pointer */

	LVBI_C, /* arg: index to const table (f.ex. string) */
	LVBI_ARG, /* arg: index */
	LVBI_LOC, /* arg: index to local variable table */
	LVBI_IV, /* push instance variable, arg: index */
	LVBI_CV, /* push class variable, arg: index */
	LVBI_CVE, /* cv external. push class variable, arg: index */

	LVBS_IVE, /* iv external. push instance variable, arg: index */
	LVBS_CVE,

	LD_SELF, /* push self class reference */
	LD_CLASS,

	// When storing new obj ref into a variable, we need its address on stack
	LABI_ARG, /* arg: index */
	LABI_LOC, /* arg: index to local variable table */
	LABI_IV, /* push instance variable, arg: index */
	LABI_CV, /* push class variable, arg: index */
	LABI_CVE, /* cv external. push class variable, arg: index */

	LABS_IVE, /* iv external. push instance variable, arg: index */
	LABS_CVE,

	IDX, /* return an item at the index. args: none. takes obj pointer
	 and the index from the stack */

	IDXA, /* return address of an item at index */

	NEW, /* arg: index to constant table.
	 creates a new object and returns its address */
	RET_SELF,
	LD_EXOBJ,
	ST_EXINFO,
	THROW
} bc_t;

extern const char *op_str[];

// We assume that struct alignment is the same on 32-bit and 64-bit systems
// This is dangerous. We should use appropriate __attribute__ to ensure compatibility

typedef enum _constant_type {
	KEK_NIL, KEK_INT, KEK_STR, KEK_SYM, KEK_ARR, KEK_EXINFO
} constant_type_t;

typedef struct _header {
	constant_type_t t;
	uint64_t cls; /* Each object needs a pointer to its class - resolved at runtime. */

	bool copied;
	void *forwarding_address;
	int survived;
	size_t size;
} header_t;

typedef struct _constant_nil {
	header_t h;
} constant_nil_t;

typedef struct _constant_int {
	header_t h;
	int value;
} constant_int_t;

typedef struct _constant_string {
	header_t h;
	int length;
	char string[1];
} constant_string_t;

typedef struct _constant_array {
	header_t h;
	int length;
	int alloc_size;
	// This is a little hack: at runtime, the last two members will be replaced
	//     by pointer to memory allocated elsewhere.
	// Padding is here in case of 64-bit pointers and an array with one element.
	// The elements won't be inlined because the type is mutable but we need to
	//     keep pointer to the array object constant
	// when reallocating the array contents somewhere else (when we grow the
	//     array for example).
	uint32_t padding;
	// This member contains offsets to the array elements (as stored in the
	//     constant table). The array of offsets is inlined.
	uint32_t elems[1];
	// When we load the array object, we allocate array of pointers and then
	//     store the actual element pointers in it.
	// The pointers will be computed from offsets and constant table location 
	//     in memory (after it's loaded).
} constant_array_t;

typedef struct _try_range {
	int try_addr;
	int catch_addr;
} try_range_t;

typedef struct _constant_exinfo {
	header_t h;
	uint32_t obj_thrown;
	uint32_t padding;
	int length;
	try_range_t blocks[1];
} constant_exinfo_t;

typedef union _constant_item {
	constant_type_t type;
	constant_int_t ci;
	constant_string_t cs;
	constant_array_t ca;
	constant_exinfo_t cei;
} constant_item_t;

/******************************************************************************/
/* classes */

typedef struct _symbol {
	uint32_t name;
	// char[name]

	DruhId type;
	Scope scope;
	uint32_t const_ptr;
} symbol_t;

typedef struct _method {
	uint32_t name;
	// char[name]

	uint32_t args;
	// symbol_t[args]

	uint32_t syms;
	// symbol_t[syms]

	uint32_t bc_entrypoint;
	uint8_t is_static;
} method_t;

typedef struct _class {
	uint32_t name;
	// char[name]

	uint32_t parent;
	// char[parent]

	uint32_t syms_static;
	// symbol_t[syms]

	uint32_t syms_method;
	// symbol_t[syms]

	uint8_t has_static_init;
	//class_method_t[0 or 1]

	uint8_t has_contructor;
	//class_method_t[0 or 1]

	uint32_t methods;
	// class_method_t[methods]
} class_t;

typedef struct _classout_wrapp {
	uint8_t *classout; /* in real: an array of uint8_t */
	size_t classout_size;
	uint32_t classout_cnt;

	uint32_t classes;
} classout_wrapp_t;

/* to serialize class hierarchy, call this function with the top class */

#define SIZE_PLACEHOLDER 666
classout_wrapp_t *classout_wrapp_init(ClassEnv *);

/* helper functions */
void classout_class(classout_wrapp_t *, ClassEnv *);
void classout_method(classout_wrapp_t *, ClassEnv *);
void classout_symbol(classout_wrapp_t *, ClassEnv *);
//classout_t *classout_init(ClassEnv *);
//class_t *class_init(ClassEnv *);
//class_t *method_init(MethodEnv *);
//symbol_t *symbol_init(PrvekTab *);

/*
 just use free(.). structs are flat
 void classout_free(classout_t *);
 void class_free(class_t *);
 void method_free(method_t *);
 void symbol_free(symbol_t *);
 */

/******************************************************************************/

typedef struct _bcout {

	size_t bc_arr_cnt;
	size_t bc_arr_size;
	uint8_t *bc_arr; /* bytecode array */
	uint8_t *bc_lab; /* bytecode labels - for internal use only, won't be saved
	 in the binary */

	size_t const_table_cnt;
	size_t const_table_size;
	uint8_t *const_table;

	int items_cnt;
	int items_size;
	constant_item_t **items; /* this will point to const_table */

} bcout_t;

/* todo, drzet si co uz tam mam, abych mohl rict, ze uz to tam je */
/* todo, interface pridej do tabulky constant a vrat offset a pridej instrukci */

bcout_t *bcout_init();
void bcout_free(bcout_t *bco);

/******************************************************************************/

/*
 00 - byte
 01 - word
 10 - double word
 11 - int
 */

/* bco write (args: byte) */
uint32_t bco_w0(bcout_t *bco, bc_t bc);
uint32_t bco_wb1(bcout_t *bco, bc_t bc, uint8_t arg);
uint32_t bco_wb2(bcout_t *bco, bc_t bc, uint8_t arg0, uint8_t arg1);

/* bco write (args: word) */
uint32_t bco_ww1(bcout_t *bco, bc_t bc, uint16_t arg);
uint32_t bco_ww1_labeled(bcout_t *bco, bc_t bc, uint16_t arg, label_t lab);
uint32_t bco_ww2(bcout_t *bco, bc_t bc, uint16_t arg0, uint16_t arg1);

/* bco write (args: double word) */
uint32_t bco_wd1(bcout_t *bco, bc_t bc, uint32_t arg);
uint32_t bco_wd2(bcout_t *bco, bc_t bc, uint32_t arg0, uint32_t arg1);

/* save a constant and get its offset */
uint32_t bco_nil(bcout_t *bco);
uint32_t bco_int(bcout_t *bco, int v);
uint32_t bco_str(bcout_t *bco, const char *str);
uint32_t bco_sym(bcout_t *bco, const char *str);
uint32_t bco_arr(bcout_t *bco, size_t len);
void bco_arr_set_idx(bcout_t *bco, uint32_t arr, size_t idx, uint32_t elem);
uint32_t bco_exinfo(bcout_t *bco, size_t try_block_cnt);
void bco_exinfo_add_block(bcout_t *bco, uint32_t exinfo, int try_addr,
		int catch_addr);

/* helper functions */
size_t bco_get_ip(bcout_t *bco);
void bco_fix_forward_jmpw(bcout_t *bco, size_t idx);
void bco_resolve_break(bcout_t *bco, size_t a1, size_t a2);

/******************************************************************************/
/* innner helper functions */
uint32_t bco_find_int(bcout_t *bco, int i);
uint32_t bco_find_str(bcout_t *bco, const char *str);
uint32_t bco_find_sym(bcout_t *bco, const char *str);
void bco_debug(const char *format, ...);
void bco_print_const(bcout_t *bco, uint32_t idx);
/******************************************************************************/

extern bcout_t *bcout_g;

void bcout_to_file(bcout_t *bcout, ClassEnv *ce, const char *filename);

#endif /* COMPILER_BCOUT_H_ */
