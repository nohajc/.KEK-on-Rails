/* zaspoc.h */

#ifndef _ZASPOC_
#define _ZASPOC_

#include "bcout.h"



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

int Gener(bc_t, int = 0, Label lab = NIL);
void resolveBreak(int a1, int a2);
void GenTR(char*);
void PutIC(int);
int GetIC();
void Print();
void Run();

#endif
