/* parser.h */

#ifndef PARSER_H
#define PARSER_H

#include "strom.h"
#include "parser.h"
#include "lexan.h"
#include "tabsym.h"

Prog *Program();
void Dekl();
void DeklKonst();
void ZbDeklKonst();
void DeklProm();
void ZbDeklProm();
StatmList *SlozPrikaz();
StatmList *ZbPrikazu();
Statm *Prikaz();
Statm *CastElse();
Expr *Podminka();
Operator RelOp();
Expr *Vyraz();
Expr *ZbVyrazu(Expr*);
Expr *Term();
Expr *ZbTermu(Expr*);
Expr *Faktor();

static LexicalSymbol Symb;

Prog* Program();
int initParser(char*);

void Srovnani(LexSymbolType);
void Srovnani_IDENT(char *);

Expr * RecordFaktor(char *);
CRecord * Record();
CRecord * ZbRecord();
void Typ(char *);
void TypVar(char *);
void TypRec();

CaseBlock * ntCASE_BODY();
CaseBlockScope * ntCASE_SCOPE();
Numb * ntCASE_RANGE();
CaseBlockScope * ntCASE_SCOPE_NEXT();

#endif
