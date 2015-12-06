/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef BC_H_
#define BC_H_

#include "stack.h"

/*
 * TODO: We should refactor our code a little to avoid duplication
 * of headers common to the compiler and the vm.
 */

typedef enum _op {
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
} op_t;
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
	THROW,
	POP
} bc_t;

#endif /* BC_H_ */