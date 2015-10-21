/* parser.cpp */

#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ChybaSrovnani(LexSymbolType s) {
	printf("chyba pri srovnani, ocekava se %s.\n", symbTable[s]);
	exit(1);
}

void ChybaExpanze(char* neterminal, LexSymbolType s) {
	printf("Chyba pri expanzi neterminalu %s, nedefinovany symbol %s.\n",
			neterminal, symbTable[s]);
	exit(1);
}

void Srovnani(LexSymbolType s) {
	if (Symb.type == s)
		Symb = readLexem();
	else
		ChybaSrovnani(s);
}

void Srovnani_IDENT(char *id) {
	if (Symb.type == IDENT) {
		strcpy(id, Symb.ident);
		Symb = readLexem();
	} else
		ChybaSrovnani(IDENT);
}

void Srovnani_NUMB(int *h) {
	if (Symb.type == NUMB) {
		*h = Symb.number;
		Symb = readLexem();
	} else
		ChybaSrovnani(NUMB);
}

Prog *Program() {
	Dekl();
	return new Prog(SlozPrikaz());
}

void Dekl() {
	switch (Symb.type) {
	case kwVAR:
		DeklProm();
		Dekl();
		break;
	case kwCONST:
		DeklKonst();
		Dekl();
		break;
	default:
		break;
	}
}

void DeklKonst() {
	char id[MAX_IDENT_LEN];
	int hod;
	Symb = readLexem();
	Srovnani_IDENT(id);
	Srovnani(EQ);
	Srovnani_NUMB(&hod);
	deklKonst(id, hod);
	ZbDeklKonst();
	Srovnani(SEMICOLON);
}

void ZbDeklKonst() {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		int hod;
		Symb = readLexem();
		Srovnani_IDENT(id);
		Srovnani(EQ);
		Srovnani_NUMB(&hod);
		deklKonst(id, hod);
		ZbDeklKonst();
	}
}

void TypRec() {
	if (Symb.type == COLON) {
		Srovnani(COLON);
		Srovnani(kwINTEGER);
	}
}

CRecord *ZbRecord() {
	if (Symb.type == COMMA) {
		Srovnani(COMMA);

		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);

		TypRec();

		return new CRecord(id, ZbRecord());
	}
	return NULL;
}

CRecord * Record() {
	char id[MAX_IDENT_LEN];
	Srovnani_IDENT(id);
	TypRec();
	CRecord *record = new CRecord(id, ZbRecord());
	Srovnani(kwEND);
	return record;
}

void TypVar(char *id) {
	switch (Symb.type) {
	case kwINTEGER:
		Srovnani(kwINTEGER);
		deklProm(id);
		break;
	case kwRECORD:
		Srovnani(kwRECORD);
		deklRecord(id, Record());
		break;
	default:
		ChybaExpanze("DeklProm", Symb.type);
		break;
	}
}

void Typ(char *id) {
	if (Symb.type == COLON) {
		Srovnani(COLON);

		TypVar(id);
	} else {
		deklProm(id);
	}
}

void DeklProm() {
	char id[MAX_IDENT_LEN];
	Symb = readLexem();
	Srovnani_IDENT(id);
	Typ(id);
	ZbDeklProm();
	Srovnani(SEMICOLON);
}

void ZbDeklProm() {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		Symb = readLexem();
		Srovnani_IDENT(id);
		Typ(id);
		ZbDeklProm();
	}
}

StatmList * SlozPrikaz() {
	Srovnani(kwBEGIN);
	Statm *p = Prikaz();
	StatmList *su = new StatmList(p, ZbPrikazu());
	Srovnani(kwEND);
	return su;
}

StatmList * ZbPrikazu() {
	if (Symb.type == SEMICOLON) {
		Symb = readLexem();
		Statm *p = Prikaz();
		return new StatmList(p, ZbPrikazu());
	}
	return 0;
}

void ZbFor(char id[MAX_IDENT_LEN], Expr * offset, Expr ** cond, Statm ** counter, Statm ** body){
	Operator op = Error, op_c = Error;
	if(Symb.type == kwTO){
		op = LessOrEq;
		op_c = Plus;
	}
	else if(Symb.type == kwDOWNTO){
		op = GreaterOrEq;
		op_c = Minus;
	}

	switch(Symb.type){
	case kwTO:
	case kwDOWNTO:
	{
		Symb = readLexem();
		*cond = new Bop(op, VarOrConst(id, offset), Vyraz());
		*counter = new Assign(new Var(adrProm(id), offset, false), new Bop(op_c, VarOrConst(id, offset), new Numb(1)));
		Srovnani(kwDO);
		*body = Prikaz();
		return;
	}

	default:
		ChybaExpanze("ZbFor", Symb.type);
		return;
	}
}

Expr * ArrayOffset(char * id) {
   if (Symb.type == LBRAC) {
      int offset = prvniIdxProm(id);

      Symb = readLexem();
      Expr * e = Vyraz();
      if (offset) {
         e = new Bop(Minus, e, new Numb(offset));
      }

      Srovnani(RBRAC);
      return e;
   }

   return NULL;
}

Statm * Prikaz() {
	Var * var;
	switch (Symb.type) {
	case IDENT: {
		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);
		Var *var = new Var(adrProm(id), ArrayOffset(id), false);
		Srovnani(ASSIGN);
		return new Assign(var, Vyraz());
	}
	case kwWRITE:
		Symb = readLexem();
		return new Write(Vyraz());
	case kwREAD:
		Symb = readLexem();
		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);
		var = new Var(adrProm(id), ArrayOffset(id), false);
		return new Read(var);
	case kwIF: {
		Symb = readLexem();
		Expr *cond = Podminka();
		Srovnani(kwTHEN);
		Statm *prikaz = Prikaz();
		return new If(cond, prikaz, CastElse());
	}
	case kwWHILE: {
		Expr *cond;
		Symb = readLexem();
		cond = Podminka();
		Srovnani(kwDO);
		return new While(cond, Prikaz());
	}
	case kwFOR: {
		Symb = readLexem();
		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);

		Expr * offset = ArrayOffset(id);
		Var *var = new Var(adrProm(id), offset, false);
		Srovnani(ASSIGN);
		Statm * init = new Assign(var, Vyraz());
		Expr * cond;
		Statm * counter;
		Statm * body;
		ZbFor(id, offset, &cond, &counter, &body);
		return new For(init, cond, counter, body);
	}


	case kwCASE: {

		Srovnani(kwCASE);
		Expr *expr = Vyraz();
		Srovnani(kwOF);
		return new Case(expr, ntCASE_BODY());
	}
	case kwBEGIN:
		return SlozPrikaz();
	default:
		return new Empty;
	}
}

Statm * CastElse() {
	if (Symb.type == kwELSE) {
		Symb = readLexem();
		return Prikaz();
	}
	return 0;
}

Expr * Podminka() {
	Expr *left = Vyraz();
	Operator op = RelOp();
	Expr *right = Vyraz();
	return new Bop(op, left, right);
}

Operator RelOp() {
	switch (Symb.type) {
	case EQ:
		Symb = readLexem();
		return Eq;
	case NEQ:
		Symb = readLexem();
		return NotEq;
	case LT:
		Symb = readLexem();
		return Less;
	case GT:
		Symb = readLexem();
		return Greater;
	case LTE:
		Symb = readLexem();
		return LessOrEq;
	case GTE:
		Symb = readLexem();
		return GreaterOrEq;
	default:
		ChybaExpanze("RelOp", Symb.type);
		return Error;
	}
}

Expr * Vyraz() {
	if (Symb.type == MINUS) {
		Symb = readLexem();
		return ZbVyrazu(new UnMinus(Term()));
	}
	return ZbVyrazu(Term());
}

Expr * ZbVyrazu(Expr * du) {
	switch (Symb.type) {
	case PLUS:
		Symb = readLexem();
		return ZbVyrazu(new Bop(Plus, du, Term()));
	case MINUS:
		Symb = readLexem();
		return ZbVyrazu(new Bop(Minus, du, Term()));
	default:
		return du;
	}
}

Expr * Term() {
	return ZbTermu(Faktor());
}

Expr * ZbTermu(Expr * du) {
	switch (Symb.type) {
	case TIMES:
		Symb = readLexem();
		return ZbTermu(new Bop(Times, du, Faktor()));
	case DIVIDE:
		Symb = readLexem();
		return ZbVyrazu(new Bop(Divide, du, Faktor()));
	default:
		return du;
	}
}

/*Expr * RecordFaktor(char *id) {
	PrvekTab *prvek = hledejId(id);
	char idMember[MAX_IDENT_LEN];
	Srovnani_IDENT(idMember);
	for (CRecord *record = prvek->record; record != NULL;
			record = record->m_Next) {
		if (strcmp(record->m_Ident, idMember) == 0) {
			return new Var(record->m_Val, true);
		}
	}
	fprintf(stderr, "Unknown record member!\n");
	exit(1);
}*/

Expr * Faktor() {
	Expr * offset;

	switch (Symb.type) {
	case IDENT:
		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);
		offset = ArrayOffset(id);
		if (Symb.type == DOT) {
			Srovnani(DOT);
			//return RecordFaktor(id);
			ChybaExpanze("Faktor", Symb.type);
		}
		return VarOrConst(id, offset);
	case NUMB:
		int hodn;
		Srovnani_NUMB(&hodn);
		return new Numb(hodn);
	case LPAR: {
		Symb = readLexem();
		Expr *su = Vyraz();
		Srovnani(RPAR);
		return su;
	}
	default:
		ChybaExpanze("Faktor", Symb.type);
		return 0;
	}
}

CaseBlock * ntCASE_BODY() {

	switch (Symb.type) {
	case NUMB: {
		CaseBlockScope *scope = ntCASE_SCOPE();
		Statm *statm = Prikaz();
		Srovnani(SEMICOLON);
		CaseBlock *caseBlock = new CaseBlock(statm, ntCASE_BODY(), scope);
		return caseBlock;
	}
	case kwELSE: {
		Srovnani(kwELSE);
		Statm *statm = Prikaz();
		Srovnani(SEMICOLON);
		Srovnani(kwEND);
		return new CaseBlock(statm, NULL, new CaseBlockScope(NULL));
	}
	case kwEND:
		Srovnani(kwEND);
		return new CaseBlock();
	default:
		ChybaExpanze("ntCASE_BODY", Symb.type);
		break;
	}

	return NULL;
}

CaseBlockScope * ntCASE_SCOPE() {
	int loeq_int;
	Srovnani_NUMB(&loeq_int);
	Numb *loeq = new Numb(loeq_int);
	Numb *hi = ntCASE_RANGE();

	if (hi == NULL) { /* number */
		return new CaseBlockScope(ntCASE_SCOPE_NEXT(), loeq);
	} else { /* range */
		return new CaseBlockScope(ntCASE_SCOPE_NEXT(), loeq, hi);
	}
}

Numb * ntCASE_RANGE() {
	switch (Symb.type) {
	case DOUBLE_DOT:
		Srovnani(DOUBLE_DOT);
		int hi;
		Srovnani_NUMB(&hi);
		return new Numb(hi);
	default:
		return NULL;
	}
}

CaseBlockScope * ntCASE_SCOPE_NEXT() {
	switch (Symb.type) {
	case COLON:
		Srovnani(COLON);
		return NULL;
	case COMMA:
		Srovnani(COMMA);
		return ntCASE_SCOPE();
	default:
		ChybaExpanze("ntCASE_SCOPE_NEXT", Symb.type);
		break;
	}
	return NULL;
}

int initParser(char *fileName) {
	if (!initLexan(fileName))
		return 0;
	Symb = readLexem();
	return 1;
}
