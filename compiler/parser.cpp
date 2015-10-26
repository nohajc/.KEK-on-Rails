/* parser.cpp */

#include "parser.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ChybaDekl(){
   printf("Chyba pri deklaraci pole, ocekava se konstantni vyraz.\n");
   exit(1);
}

void ChybaSrovnani(LexSymbolType s) {
	printf("chyba pri srovnani, ocekava se %s.\n", symbTable[s]);
	exit(1);
}

void ChybaExpanze(char* neterminal, LexSymbolType s) {
	printf("Chyba pri expanzi neterminalu %s, nedefinovany symbol %s.\n",
			neterminal, symbTable[s]);
	exit(1);
}

void Chyba(char * msg) {
	printf("%s\n", msg);
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

static void skipNewlines() {
	while (Symb.type == NEWLINE){
		Symb = readLexem();
	}
}

Prog *Program() {
	//Dekl(); // Declarations moved inside the program. They can be anywhere.
	//return new Prog(SlozPrikaz());
	return new Prog(SeznamTrid());
}

StatmList * Dekl(Env env) {
	StatmList * p, * d, * k;
	switch (Symb.type) {
	case kwVAR:
		p = DeklProm(env);
		d = Dekl(env);
		return new StatmList(p, d);
	case kwCONST:
		k = DeklKonst(env);
		d = Dekl(env);
		return new StatmList(k, d);
	default:
		return new StatmList(new Empty, NULL);
	}
}

StatmList * DeklKonst(Env env) {
	char id[MAX_IDENT_LEN];
	int hod;
	Symb = readLexem();
	Srovnani_IDENT(id);
	Srovnani(ASSIGN);
	Srovnani_NUMB(&hod);
	deklKonst(id, hod);
	ZbDeklKonst(env);
	if(Symb.type == SEMICOLON){
		Srovnani(SEMICOLON);
	}
	else if(Symb.type == NEWLINE){
		Srovnani(NEWLINE);
	}
	return new StatmList(new Empty, NULL);
}

void ZbDeklKonst(Env env) {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		int hod;
		Symb = readLexem();
		Srovnani_IDENT(id);
		Srovnani(ASSIGN);
		Srovnani_NUMB(&hod);
		deklKonst(id, hod);
		ZbDeklKonst(env);
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
	Srovnani(RCURLY);
	return record;
}

/*void TypVar(char *id) {
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
}*/

// Returns true if the Type is primitive (integer, char, ...)
bool Typ(Env env, char *id) { // TODO: remove static array declaration
	/*if (Symb.type == COLON) {
		Srovnani(COLON);

		TypVar(id);
	} else {
		deklProm(id);
	}*/
	if(Symb.type == LBRAC){
      Numb *n_prvni, *n_posledni;

      Symb = readLexem();
      //Srovnani_NUMB(&prvni);
      n_prvni = dynamic_cast<Numb*>(Vyraz()->Optimize());
      if(!n_prvni) ChybaDekl();

      Srovnani(DOUBLE_DOT);

      //Srovnani_NUMB(&posledni);
      n_posledni = dynamic_cast<Numb*>(Vyraz()->Optimize());
      if(!n_posledni) ChybaDekl();
      
      Srovnani(RBRAC);
      deklProm(id, n_prvni->Value(), n_posledni->Value());
      delete n_prvni;
      delete n_posledni;
      return false;
   }
   else{
      deklProm(id);
      return true;
   }
}

StatmList * DeklProm(Env env) {
	char id[MAX_IDENT_LEN];
	Symb = readLexem();
	Srovnani_IDENT(id);
	bool prim = Typ(env, id);
	Statm * st;
	StatmList * ret;
	Var * var;
	Expr * e;

	if (prim && Symb.type == ASSIGN) {
		Symb = readLexem();
		var = new Var(id, adrProm(id), NULL, false);
		e = Vyraz();
		st = new Assign(var, e);
	}
	else {
		st = new Empty;
	}

	ret = new StatmList(st, ZbDeklProm(env));
	if (Symb.type == SEMICOLON) {
		Srovnani(SEMICOLON);
	}
	else if (Symb.type == NEWLINE) {
		Srovnani(NEWLINE);
	}

	return ret;
}

StatmList * ZbDeklProm(Env env) {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		Symb = readLexem();
		Srovnani_IDENT(id);
		bool prim = Typ(env, id);
		Statm * st;
		Var * var;
		Expr * e;

		if (prim && Symb.type == ASSIGN) {
			Symb = readLexem();
			var = new Var(id, adrProm(id), NULL, false);
			e = Vyraz();
			st = new Assign(var, e);
		}
		else {
			st = new Empty;
		}

		return new StatmList(st, ZbDeklProm(env));
	}
	return new StatmList(new Empty, NULL);
}

ClassList * SeznamTrid() {
	Class * cls = Trida();
	ClassList * ret = NULL;

	if (cls) {
		ret = new ClassList(cls, ZbTrid());
		Srovnani(EOI);
	}
	return ret;
}

ClassList * ZbTrid() {
	if (Symb.type != EOI) {
		if (Symb.type == SEMICOLON || Symb.type == NEWLINE) {
			Symb = readLexem();
		}
		Class * cls = Trida();
		if(!cls){
			return NULL;
		}
		return new ClassList(cls, ZbTrid());
	}
	return 0;
}

Class * Trida() {
	char id[MAX_IDENT_LEN], parId[MAX_IDENT_LEN];
	Env env;
	ClassEnv * clsEnv;
	Class * ret;

	skipNewlines();
	if(Symb.type == EOI){
		return NULL;
	}

	Srovnani(kwCLASS);
	Srovnani_IDENT(id);
	if(Symb.type == COLON){ // Class has parent
		Symb = readLexem();
		Srovnani_IDENT(parId);
		clsEnv = deklClass(id, parId);
	}
	else{ // Class does not have a parent
		clsEnv = deklClass(id);
	}

	env.clsEnv = clsEnv;
	env.mthEnv = NULL;
	return new Class(SeznamMetod(env));
}

StatmList * SeznamMetod(Env env) {
	Srovnani(LCURLY);
	Statm * m = ClenTridy(env);
	StatmList * lst = new StatmList(m, ZbMetod(env));
	Srovnani(RCURLY);
	return lst;
}

StatmList * ZbMetod(Env env) {
	if (Symb.type != RCURLY) {
		if (Symb.type == SEMICOLON || Symb.type == NEWLINE) {
			Symb = readLexem();
		}
		Statm * m = ClenTridy(env);
		if(!m){
			return NULL;
		}
		return new StatmList(m, ZbMetod(env));
	}
	return 0;
}

Statm * ClenTridy(Env env) {
	bool isStatic = false;

	skipNewlines();
	if(Symb.type == RCURLY){
		return NULL;
	}

	if (Symb.type == kwSTATIC) {
		isStatic = true;
		Symb = readLexem();
	}

	switch (Symb.type) {
	case kwVAR:
	case kwCONST:
		return Dekl(env);
	default:
		return Metoda(env, isStatic);
	}
}

Statm * Metoda(Env env, bool isStatic) {
	char id[MAX_IDENT_LEN];
	bool isConstructor = false;

	Srovnani_IDENT(id);

	if(env.clsEnv && !strcmp(id, env.clsEnv->className)){
		isConstructor = true;
		//printf("Found constructor %s.\n", id);
	}

	if(isConstructor && isStatic) {
		Chyba("Konstruktor nesmi byt staticky.");
	}

	MethodEnv * mthEnv = deklMethod(id, isConstructor, isStatic, env.clsEnv); // TODO: Pass down env information
	env.mthEnv = mthEnv;

	Srovnani(LPAR);
	// Parse method args
	while (Symb.type != RPAR) {
		Srovnani_IDENT(id);
		deklProm(id, true);
		if (Symb.type != RPAR) {
			Srovnani(COMMA);
		}
	}
	Srovnani(RPAR);

	return new Method(SlozPrikaz(env)); // TODO: pass down env information
}

StatmList * SlozPrikaz(Env env, Context ctxt) {
	Srovnani(LCURLY);
	Statm *p = Prikaz(env, ctxt);
	StatmList *su = new StatmList(p, ZbPrikazu(env, ctxt));
	Srovnani(RCURLY);
	return su;
}

StatmList * ZbPrikazu(Env env, Context ctxt) {
	if (Symb.type != RCURLY) {
		if (Symb.type == SEMICOLON || Symb.type == NEWLINE) {
			Symb = readLexem();
		}
		Statm *p = Prikaz(env, ctxt);
		return new StatmList(p, ZbPrikazu(env, ctxt));
	}
	return 0;
}

void ZbFor(Env env, char id[MAX_IDENT_LEN], Expr * offset, Expr ** cond, Statm ** counter, Statm ** body){
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
		*counter = new Assign(new Var(id, adrProm(id), offset, false), new Bop(op_c, VarOrConst(id, offset), new Numb(1)));
		Srovnani(RPAR);
		break;
	}
	case SEMICOLON:
		Symb = readLexem();
		*cond = Podminka();
		Srovnani(SEMICOLON);
		*counter = Assignment();
		Srovnani(RPAR);
		break;
	default:
		ChybaExpanze("ZbFor", Symb.type);
		return;
	}
	*body = Prikaz(env, C_CYCLE);
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

Statm * Assignment() {
	char id[MAX_IDENT_LEN];

	Srovnani_IDENT(id);
	Expr * offset = ArrayOffset(id);
	Var *var = new Var(id, adrProm(id), offset, false);
	Expr * e;
	//Srovnani(ASSIGN);
	switch (Symb.type) {
	case ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, e);
	case ADD_ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, new Bop(Plus, VarOrConst(id, offset), e));
	case SUB_ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, new Bop(Minus, VarOrConst(id, offset), e));
	case MUL_ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, new Bop(Times, VarOrConst(id, offset), e));
	case DIV_ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, new Bop(Divide, VarOrConst(id, offset), e));
	case MOD_ASSIGN:
		Symb = readLexem();
		e = Vyraz();
		return new Assign(var, new Bop(Modulo, VarOrConst(id, offset), e));
	case INCREMENT:
		Symb = readLexem();
		return new Assign(var, new Bop(Plus, VarOrConst(id, offset), new Numb(1)));
	case DECREMENT:
		Symb = readLexem();
		return new Assign(var, new Bop(Minus, VarOrConst(id, offset), new Numb(1)));
	default:
		ChybaExpanze("Assignment", Symb.type);
		return NULL;
	}
}

Statm * Prikaz(Env env, Context ctxt) {
	Var * var;
	skipNewlines();

	switch (Symb.type) {
	case kwVAR:
	case kwCONST:
		return Dekl(env);
	case kwRETURN:
		Symb = readLexem();
		return new Return(Vyraz());
	case IDENT: {
		return Assignment();
		/*char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);
		Var *var = new Var(adrProm(id), ArrayOffset(id), false);
		Srovnani(ASSIGN);
		return new Assign(var, Vyraz());*/
	}
	case kwWRITE:
		Symb = readLexem();
		return new Write(Vyraz());
	case kwREAD:
		Symb = readLexem();
		char id[MAX_IDENT_LEN];
		Srovnani_IDENT(id);
		var = new Var(id, adrProm(id), ArrayOffset(id), false);
		return new Read(var);
	case kwIF: {
		Symb = readLexem();
		Srovnani(LPAR);
		Expr *cond = Podminka();
		Srovnani(RPAR);
		Statm *prikaz = Prikaz(env, ctxt);
		return new If(cond, prikaz, CastElse(env, ctxt));
	}
	case kwWHILE: {
		Expr *cond;
		Symb = readLexem();
		Srovnani(LPAR);
		cond = Podminka();
		Srovnani(RPAR);
		return new While(cond, Prikaz(env, C_CYCLE));
	}
	case kwFOR: {
		char id[MAX_IDENT_LEN];
		Symb = readLexem();
		Srovnani(LPAR);
		Srovnani_IDENT(id);

		Expr * offset = ArrayOffset(id);
		Var *var = new Var(id, adrProm(id), offset, false);
		Srovnani(ASSIGN);
		Statm * init = new Assign(var, Vyraz());
		Expr * cond;
		Statm * counter;
		Statm * body;
		ZbFor(env, id, offset, &cond, &counter, &body);
		return new For(init, cond, counter, body);
	}
	case kwBREAK: {
		Symb = readLexem();
		if(ctxt == C_CYCLE){
			return new Break();
		}
		Chyba("Prikaz break je platny pouze uvnitr cyklu nebo prikazu switch.");
		return new Empty;
	}

	case kwSWITCH: {
		Srovnani(kwSWITCH);
		Srovnani(LPAR);
		Expr *expr = Vyraz();
		Srovnani(RPAR);
		Srovnani(LCURLY);
		return new Case(expr, ntCASE_BODY(env));
	}
	case LCURLY:
		return SlozPrikaz(env, ctxt);
	default:
		return new Empty;
	}
}

Statm * CastElse(Env env, Context ctxt) {
	skipNewlines();

	if (Symb.type == kwELSE) {
		Symb = readLexem();
		return Prikaz(env, ctxt);
	}
	return 0;
}

Expr * Podminka() {
	/*Expr *left = Vyraz();
	Operator op = RelOp();
	Expr *right = Vyraz();
	return new Bop(op, left, right);*/
	return Vyraz();
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
		//ChybaExpanze("RelOp", Symb.type);
		return Error;
	}
}

Expr * Vyraz() {
	return ZbVyrazu(LOrTerm());
}

Expr * ZbVyrazu(Expr * du) {
	switch (Symb.type) {
	case LOG_OR:
		Symb = readLexem();
		return ZbVyrazu(new Bop(LogOr, du, LOrTerm()));
	default:
		return du;
	}
}

Expr * LOrTerm() {
	return ZbLOrTermu(XorTerm());
}

Expr * ZbLOrTermu(Expr * du) {
	switch (Symb.type) {
	case XOR:
		Symb = readLexem();
		return ZbLOrTermu(new Bop(Xor, du, XorTerm()));
	default:
		return du;
	}
}

Expr * XorTerm() {
	return ZbXorTermu(LAndTerm());
}

Expr * ZbXorTermu(Expr * du) {
	switch (Symb.type) {
	case LOG_AND:
		Symb = readLexem();
		return ZbXorTermu(new Bop(LogAnd, du, LAndTerm()));
	default:
		return du;
	}
}

Expr * LAndTerm() {
	return ZbLAndTermu(BOrTerm());
}

Expr * ZbLAndTermu(Expr * du) {
	switch (Symb.type) {
	case BIT_OR:
		Symb = readLexem();
		return ZbLAndTermu(new Bop(BitOr, du, BOrTerm()));
	default:
		return du;
	}
}

Expr * BOrTerm() {
	return ZbBOrTermu(BAndTerm());
}

Expr * ZbBOrTermu(Expr * du) {
	switch (Symb.type) {
	case BIT_AND:
		Symb = readLexem();
		return ZbBOrTermu(new Bop(BitAnd, du, BAndTerm()));
	default:
		return du;
	}
}

Expr * BAndTerm() {
	return ZbBAndTermu(RelOpTerm());
}

Expr * ZbBAndTermu(Expr * du) {
	Operator op = RelOp();
	if (op == Error) {
		return du;
	}
	return ZbBAndTermu(new Bop(op, du, RelOpTerm()));
}

Expr * RelOpTerm() {
	return ZbRelOpTermu(ShiftTerm());
}

Expr * ZbRelOpTermu(Expr * du) {
	switch (Symb.type) {
	case LSHIFT:
		Symb = readLexem();
		return ZbRelOpTermu(new Bop(Lsh, du, ShiftTerm()));
	case RSHIFT:
		Symb = readLexem();
		return ZbRelOpTermu(new Bop(Rsh, du, ShiftTerm()));
	default:
		return du;
	}
}

Expr * ShiftTerm() {
	return ZbShiftTermu(Term());
}

Expr * ZbShiftTermu(Expr * du) {
	switch (Symb.type) {
	case PLUS:
		Symb = readLexem();
		return ZbShiftTermu(new Bop(Plus, du, Term()));
	case MINUS:
		Symb = readLexem();
		return ZbShiftTermu(new Bop(Minus, du, Term()));
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
	case MODULO:
		Symb = readLexem();
		return ZbVyrazu(new Bop(Modulo, du, Faktor()));
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

	if (Symb.type == MINUS) {
		Symb = readLexem();
		return new UnMinus(Faktor());
	}
	if (Symb.type == LOG_NOT) {
		Symb = readLexem();
		return new Not(Faktor());
	}

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

CaseBlock * ntCASE_BODY(Env env) {
	skipNewlines();

	switch (Symb.type) {
	case kwCASE: {
		Symb = readLexem();
		CaseBlockScope *scope = ntCASE_SCOPE();
		Statm *statm = Prikaz(env);
		if(Symb.type == SEMICOLON){
			Srovnani(SEMICOLON);
		}
		else{
			Srovnani(NEWLINE);
		}
		CaseBlock *caseBlock = new CaseBlock(statm, ntCASE_BODY(env), scope);
		return caseBlock;
	}
	case kwDEFAULT: {
		Srovnani(kwDEFAULT);
		Srovnani(COLON);
		Statm *statm = Prikaz(env);
		if(Symb.type == SEMICOLON){
			Srovnani(SEMICOLON);
		}
		else{
			Srovnani(NEWLINE);
		}
		Srovnani(RCURLY);
		return new CaseBlock(statm, NULL, new CaseBlockScope(NULL));
	}
	case RCURLY: {
		return new CaseBlock();
	}
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
