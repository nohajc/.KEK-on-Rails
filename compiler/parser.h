/* parser.h */

#ifndef PARSER_H
#define PARSER_H

#include "strom.h"
#include "parser.h"
#include "lexan.h"
#include "tabsym.h"

enum Context {
	C_NIL, C_CYCLE
};

Prog *Program();
StatmList * Dekl(Env env);
StatmList * DeklKonst(Env env);
void ZbDeklKonst(Env env);
StatmList * DeklProm(Env env);
StatmList * ZbDeklProm(Env env);
StatmList *SlozPrikaz(Env env, Context ctxt = C_NIL);
StatmList *ZbPrikazu(Env env, Context ctxt = C_NIL);
Statm *Prikaz(Env env, Context ctxt = C_NIL);
Statm *Assignment();
Statm *CastElse(Env env, Context ctxt = C_NIL);
void ZbFor(Env env, char id[MAX_IDENT_LEN], Expr * offset, Expr ** cond, Statm ** counter, Statm ** body);
Expr *Podminka();
Operator RelOp();
Expr *Vyraz();
Expr *ZbVyrazu(Expr*);
Expr * LOrTerm();
Expr * ZbLOrTermu(Expr*);
Expr * XorTerm();
Expr * ZbXorTermu(Expr * du);
Expr * LAndTerm();
Expr * ZbLAndTermu(Expr*);
Expr * BOrTerm();
Expr * ZbBOrTermu(Expr*);
Expr * BAndTerm();
Expr * ZbBAndTermu(Expr*);
Expr * RelOpTerm();
Expr * ZbRelOpTermu(Expr*);
Expr * ShiftTerm();
Expr * ZbShiftTermu(Expr * du);
Expr *Term();
Expr *ZbTermu(Expr*);
Expr *Faktor();
Expr *ZbFaktoru(Expr*);

ClassList * SeznamTrid();
ClassList * ZbTrid();
Class * Trida();
StatmList * SeznamMetod(Env env);
StatmList * ZbMetod(Env env);
Statm * ClenTridy(Env env);
Statm * Metoda(Env env, bool isStatic);

static LexicalSymbol Symb;

Prog* Program();
int initParser(char*);

void Srovnani(LexSymbolType);
void Srovnani_IDENT(char *);

Expr * RecordFaktor(char *);
CRecord * Record();
CRecord * ZbRecord();
bool Typ(Env env, char *);
void TypVar(char *);
void TypRec();

CaseBlock * ntCASE_BODY(Env env);
CaseBlockScope * ntCASE_SCOPE();
Numb * ntCASE_RANGE();
CaseBlockScope * ntCASE_SCOPE_NEXT();

#endif
