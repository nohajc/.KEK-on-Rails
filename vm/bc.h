/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef BC_H_
#define BC_H_

/*
 * TODO: We should refactor our code a little to avoid duplication
 * of headers common to the compiler and the vm.
 */

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
typedef enum _bc {
	UNDEF, /**/
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

#endif /* BC_H_ */