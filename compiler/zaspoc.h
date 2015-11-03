/* zaspoc.h */

#ifndef _ZASPOC_
#define _ZASPOC_

enum TypInstr {
	LDC, /* arg: number, load constant (f.ex. a number) */
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
	PUSH_SUPER, /* push parent's class reference */

	NEW, /* arg: index to constant table.
	        creates a new object and returns its address */
};

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
	UBRK - break with unresolved address
	BRK - break with resolved address
*/
enum Label {
	NIL, UBRK, BRK
};

int Gener(TypInstr, int = 0, Label lab = NIL);
void resolveBreak(int a1, int a2);
void GenTR(char*);
void PutIC(int);
int GetIC();
void Print();
void Run();

#endif
