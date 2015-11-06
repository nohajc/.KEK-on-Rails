/*
 * bcout.h
 *
 *  Created on: Nov 3, 2015
 *      Author: n
 */

#ifndef COMPILER_BCOUT_H_
#define COMPILER_BCOUT_H_


#define DEBUG 1

#include <stdint.h>
#include <stdio.h>


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
	NIL, BRK
} label_t;

typedef enum _bc {
	UNDEF,
	BOP, /**/
	UNM, /**/
	LD, /* load */
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

	PUSH_C, /* arg: index to const table (f.ex. string) */
	PUSH_ARG, /* arg: index */
	PUSH_LOC, /* arg: index to local variable table */
	PUSH_IV, /* push instance variable, arg: index */
	PUSH_CV, /* push class variable, arg: index */
	PUSH_IVE, /* iv external. push instance variable, arg: index */
	PUSH_CVE, /* cv external. push class variable, arg: index */
	PUSH_SELF, /* push self class reference */
	CLASSREF,

	// When storing new obj ref into a variable, we need its address on stack
	PUSHA_ARG, /* arg: index */
	PUSHA_LOC, /* arg: index to local variable table */
	PUSHA_IV, /* push instance variable, arg: index */
	PUSHA_CV, /* push class variable, arg: index */
	PUSHA_IVE, /* iv external. push instance variable, arg: index */
	PUSHA_CVE, /* cv external. push class variable, arg: index */

	IDX, /* return an item at the index. args: none. takes obj pointer
	and the index from the stack */

	IDXA, /* return address of an item at index */

	NEW /* arg: index to constant table.
	        creates a new object and returns its address */
} bc_t;

extern const char *op_str[];

typedef enum _constant_type {
	INT, STRING, SYMBOL, ARR
} constant_type_t;

typedef struct _constant_int {
	constant_type_t type;
	int value;
} constant_int_t;

typedef struct _constant_string {
	constant_type_t type;
	int length;
	char string[1];
} constant_string_t;

typedef union _constant_item {
	constant_type_t type;
	constant_int_t ci;
	constant_string_t cs;
} constant_item_t;

typedef struct _bcout {

	size_t bc_arr_cnt;
	size_t bc_arr_size;
	uint8_t *bc_arr; /* bytecode array */
	uint8_t *bc_lab; /* bytecode labels - for internal use only, won't be saved in the binary */

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
uint32_t bco_int(bcout_t *bco, int v);
uint32_t bco_str(bcout_t *bco, const char *str);
uint32_t bco_sym(bcout_t *bco, const char *str);
uint32_t bco_arr(bcout_t *bco, constant_item_t *arr);

/* helper functions */
size_t bco_get_ip(bcout_t *bco);
void bco_fix_forward_jmpw(bcout_t *bco, size_t idx);
void bco_resolve_break(bcout_t *bco, size_t a1, size_t a2);

/******************************************************************************/
/* innner helper functions */
uint32_t bco_find_int(bcout_t *bco, int i);
uint32_t bco_find_str(bcout_t *bco, const char *str);
uint32_t bco_find_sym(bcout_t *bco, const char *str);
void bco_debug(const char *format, va_list ap);
/******************************************************************************/

extern bcout_t *bcout_g;

#endif /* COMPILER_BCOUT_H_ */
