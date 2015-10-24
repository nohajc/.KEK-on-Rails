/* lexan.h */

#ifndef LEXAN_H
#define LEXAN_H

#include "vstup.h"

#define DEBUG_LEXEM 0

typedef enum {
	IDENT,
	NUMB,
	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	MODULO,
	EQ,
	NEQ,
	LT,
	GT,
	LTE,
	GTE,
	LPAR,
	RPAR,
	ASSIGN,
	COMMA,
	SEMICOLON,
	NEWLINE,
	kwVAR,
	kwCONST,
	LCURLY,
	RCURLY,
	kwIF,
	kwTHEN,
	kwELSE,
	kwWHILE,
	kwDO,
	kwWRITE,
	kwREAD,
	EOI,
	ERR,
	kwCASE,
	kwOF,
	DOT,
	DOUBLE_DOT,
	COLON,
	DASH,
	kwINTEGER,
	kwRECORD,
	LBRAC,
	RBRAC,
	kwFOR,
	kwTO,
	kwDOWNTO,
	ADD_ASSIGN,
	SUB_ASSIGN,
	MUL_ASSIGN,
	DIV_ASSIGN,
	MOD_ASSIGN,
	LOG_NOT,
	LOG_OR,
	LOG_AND,
	BIT_OR,
	BIT_AND,
	XOR
} LexSymbolType;

extern const char *symbTable[];

#define MAX_IDENT_LEN 32

typedef struct LexicalSymbol {
	LexSymbolType type;
	char ident[MAX_IDENT_LEN]; /* atribut symbolu IDENT */
	int number; /* atribut symbolu NUMB */
} LexicalSymbol;

LexicalSymbol readLexem(void);
int initLexan(char*);

#endif
