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

StatmList * Dekl(Env env, bool isStatic) {
	StatmList * p, * d, * k;
	switch (Symb.type) {
	case kwVAR:
		p = DeklProm(env, isStatic);
		d = Dekl(env, isStatic);
		return new StatmList(p, d);
	case kwCONST:
		k = DeklKonst(env, isStatic);
		d = Dekl(env, isStatic);
		return new StatmList(k, d);
	default:
		return new StatmList(new Empty, NULL);
	}
}

StatmList * DeklKonst(Env env, bool isStatic) {
	char id[MAX_IDENT_LEN];
	int hod;
	Symb = readLexem();
	Srovnani_IDENT(id);
	Srovnani(ASSIGN);
	Srovnani_NUMB(&hod);
	deklKonst(id, hod, isStatic, env.clsEnv, env.mthEnv);
	ZbDeklKonst(env, isStatic);
	if(Symb.type == SEMICOLON){
		Srovnani(SEMICOLON);
	}
	else if(Symb.type == NEWLINE){
		Srovnani(NEWLINE);
	}
	return new StatmList(new Empty, NULL);
}

void ZbDeklKonst(Env env, bool isStatic) {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		int hod;
		Symb = readLexem();
		Srovnani_IDENT(id);
		Srovnani(ASSIGN);
		Srovnani_NUMB(&hod);
		deklKonst(id, hod, isStatic, env.clsEnv, env.mthEnv);
		ZbDeklKonst(env, isStatic);
	}
}

void TypRec() {
	if (Symb.type == COLON) {
		Srovnani(COLON);
		Srovnani(kwINTEGER);
	}
}

/*CRecord *ZbRecord() {
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
}*/

// Returns true if the Type is primitive (integer, char, ...)
bool Typ(Env env, char *id, bool isStatic) { // TODO: remove static array declaration
	/*if (Symb.type == COLON) {
		Srovnani(COLON);

		TypVar(id);
	} else {
		deklProm(id);
	}*/
	if(Symb.type == LBRAC){ // TODO: Change static arrays to dynamic
      Numb *n_prvni, *n_posledni;

      Symb = readLexem();
      //Srovnani_NUMB(&prvni);
      n_prvni = dynamic_cast<Numb*>(Vyraz(env)->Optimize());
      if(!n_prvni) ChybaDekl();

      Srovnani(DOUBLE_DOT);

      //Srovnani_NUMB(&posledni);
      n_posledni = dynamic_cast<Numb*>(Vyraz(env)->Optimize());
      if(!n_posledni) ChybaDekl();
      
      Srovnani(RBRAC);
      deklProm(id, n_prvni->Value(), n_posledni->Value(), env.clsEnv, env.mthEnv);
      delete n_prvni;
      delete n_posledni;
      return false;
   }
   else{
      deklProm(id, false, isStatic, env.clsEnv, env.mthEnv);
      return true;
   }
}

StatmList * DeklProm(Env env, bool isStatic) {
	char id[MAX_IDENT_LEN];
	Symb = readLexem();
	Srovnani_IDENT(id);
	bool prim = Typ(env, id, isStatic);
	Statm * st;
	StatmList * ret;
	Var * var;
	Expr * e;

	if (prim && Symb.type == ASSIGN) {
		Symb = readLexem();
		var = new Var(adrProm(id, env.clsEnv, env.mthEnv), NULL, false);
		e = Vyraz(env);
		st = new Assign(var, e);
	}
	else {
		st = new Empty;
	}

	ret = new StatmList(st, ZbDeklProm(env, isStatic));
	if (Symb.type == SEMICOLON) {
		Srovnani(SEMICOLON);
	}
	else if (Symb.type == NEWLINE) {
		Srovnani(NEWLINE);
	}

	return ret;
}

StatmList * ZbDeklProm(Env env, bool isStatic) {
	if (Symb.type == COMMA) {
		char id[MAX_IDENT_LEN];
		Symb = readLexem();
		Srovnani_IDENT(id);
		bool prim = Typ(env, id, isStatic);
		Statm * st;
		Var * var;
		Expr * e;

		if (prim && Symb.type == ASSIGN) {
			Symb = readLexem();
			var = new Var(adrProm(id, env.clsEnv, env.mthEnv), NULL, false);
			e = Vyraz(env);
			st = new Assign(var, e);
		}
		else {
			st = new Empty;
		}

		return new StatmList(st, ZbDeklProm(env, isStatic));
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
	StatmList * lst;
	if (!m) {
		lst = NULL;
	}
	else{
		lst = new StatmList(m, ZbMetod(env));
	}
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
		return Dekl(env, isStatic);
	default:
		env.self = true; // call from method body is to self (own class methods) by default
		return Metoda(env, isStatic);
	}
}

Statm * Metoda(Env env, bool isStatic) {
	char mth_id[MAX_IDENT_LEN];
	char id[MAX_IDENT_LEN];
	bool isConstructor = false;
	int numArgs = 0;

	Srovnani_IDENT(mth_id);

	if(env.clsEnv && !strcmp(mth_id, env.clsEnv->className)){
		isConstructor = true;
		//printf("Found constructor %s.\n", mth_id);
	}

	if(isConstructor && isStatic) {
		Chyba("Konstruktor nesmi byt staticky.");
	}

	MethodEnv * mthEnv = deklMethod(mth_id, isConstructor, isStatic, env.clsEnv);
	env.mthEnv = mthEnv;

	Srovnani(LPAR);
	// Parse method args
	while (Symb.type != RPAR) {
		Srovnani_IDENT(id);
		deklProm(id, true, false, env.clsEnv, env.mthEnv);
		numArgs++;
		if (Symb.type != RPAR) {
			Srovnani(COMMA);
		}
	}
	Srovnani(RPAR);

	return new Method(mth_id, isStatic, numArgs, SlozPrikaz(env));
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
	/*Operator op = Error, op_c = Error;
	if(Symb.type == kwTO){
		op = LessOrEq;
		op_c = Plus;
	}
	else if(Symb.type == kwDOWNTO){
		op = GreaterOrEq;
		op_c = Minus;
	}*/

	switch(Symb.type){
	/*case kwTO: // BROKEN!
	case kwDOWNTO:
	{
		Symb = readLexem();
		*cond = new Bop(op, VarOrConst(id, offset, env), Vyraz(env));
		//*counter = new Assign(new Var(adrProm(id, env.clsEnv, env.mthEnv), offset, false), new Bop(op_c, VarOrConst(id, offset, env), new Numb(1)));
		*counter = new AssignWithBop(op_c, new Var(adrProm(id, env.clsEnv, env.mthEnv), offset, false), new Numb(1));
		Srovnani(RPAR);
		break;
	}*/
	case SEMICOLON:
		Symb = readLexem();
		*cond = Podminka(env);
		Srovnani(SEMICOLON);
		*counter = Assignment(env);
		Srovnani(RPAR);
		break;
	default:
		ChybaExpanze("ZbFor", Symb.type);
		return;
	}
	*body = Prikaz(env, C_CYCLE);
}

Expr * ArrayOffset(Env env, char * id) {
   if (Symb.type == LBRAC) {
      int offset = prvniIdxProm(id, env.clsEnv, env.mthEnv);

      Symb = readLexem();
      Expr * e = Vyraz(env);
      if (offset) {
         e = new Bop(Minus, e, new Numb(offset));
      }

      Srovnani(RBRAC);
      return e;
   }

   return NULL;
}

Expr * ZbIdent(Env env, bool rvalue) {
	char id[MAX_IDENT_LEN];
	PrvekTab * p;
	ClassEnv * c;
	MethodEnv * m;

	if (env.mthEnv && Symb.type == kwTHIS) { // self ref
		Symb = readLexem();

		if (Symb.type == DOT) {
			Symb = readLexem();
			env.mthEnv = NULL;
			return ZbIdent(env, rvalue);
		}
		// self ref by itself
		return new Var("this", rvalue); // Special Var. Could we do it better?
	}

	Srovnani_IDENT(id);
	Expr * offset = ArrayOffset(env, id);

	switch (Symb.type) {
	case DOT: // ref
		Symb = readLexem();
		p = hledejMember(id, env.clsEnv, env.mthEnv);
		if (!rvalue && p->druh != IdProm) {
			Chyba("Na leve strane musi byt promenna.");
		}

		if (p) { // obj ref
			env.self = false;
			env.clsEnv = CLASS_ANY;
			env.mthEnv = NULL;
			return new ObjRef(p, offset, rvalue, ZbIdent(env, rvalue));
		}

		c = hledejClass(id);
		if (c) { // class ref
			if (offset) {
				Chyba("Ke tride nelze pristupovat jako k poli");
			}
			env.self = false;
			env.clsEnv = c;
			env.mthEnv = NULL;
			return new ClassRef(id, rvalue, ZbIdent(env, rvalue));
		}
		Chyba("Ocekava se deklarovany objekt nebo trida.");
		break;
	case LPAR: // call
		m = hledejMethod(id, env.clsEnv);
		if (m) {
			if (!env.self && env.clsEnv != CLASS_ANY && !m->isStatic) {
				Chyba("Volana metoda musi byt staticka.");
			}
			if (env.self && env.mthEnv && env.mthEnv->isStatic && !m->isStatic) {
				Chyba("Instancni metodu nelze volat ze statickeho kontextu.");
			}
			return new MethodRef(id);
		}
		Chyba("Volana metoda neexistuje.");
	default: // var/const
		if (rvalue) {
			return VarOrConst(id, offset, env);
		}
		return new Var(adrProm(id, env.clsEnv, env.mthEnv), offset, false);
	}

	return NULL;
}

ArgList * ZbArgs(Env env) {
	Expr * arg;
	if(Symb.type == RPAR) {
		return NULL;
	}

	Srovnani(COMMA);
	arg = Vyraz(env);
	return new ArgList(arg, ZbArgs(env));
}

ArgList * Args(Env env) {
	Expr * arg;
	if(Symb.type == RPAR) {
		return NULL;
	}

	arg = Vyraz(env);
	return new ArgList(arg, ZbArgs(env));
}

Expr * Ident(Env env, bool rvalue) {
	Expr * e = ZbIdent(env, rvalue);

	if (Symb.type == LPAR) {
		Symb = readLexem();
		ArgList * a = Args(env);
		Srovnani(RPAR);

		return new Call(e, a);
	}

	return e;
}

Statm * Assignment(Env env, Var * lvalue) {
	Var * var;
	Expr * e;
	if (!lvalue) {
		e = Ident(env, false);
		var = dynamic_cast<Var*>(e); // var
		if (!var) {
			Chyba("Ocekava se prirazeni.");
		}
	}
	else {
		var = lvalue;
	}

	switch (Symb.type) {
	case ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		return new Assign(var, e);
	case ADD_ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		//return new Assign(var, new Bop(Plus, VarOrConst(id, offset, env), e));
		return new AssignWithBop(Plus, var, e);
	case SUB_ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		//return new Assign(var, new Bop(Minus, VarOrConst(id, offset, env), e));
		return new AssignWithBop(Minus, var, e);
	case MUL_ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		//return new Assign(var, new Bop(Times, VarOrConst(id, offset, env), e));
		return new AssignWithBop(Times, var, e);
	case DIV_ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		//return new Assign(var, new Bop(Divide, VarOrConst(id, offset, env), e));
		return new AssignWithBop(Divide, var, e);
	case MOD_ASSIGN:
		Symb = readLexem();
		e = Vyraz(env);
		//return new Assign(var, new Bop(Modulo, VarOrConst(id, offset, env), e));
		return new AssignWithBop(Modulo, var, e);
	case INCREMENT:
		Symb = readLexem();
		//return new Assign(var, new Bop(Plus, VarOrConst(id, offset, env), new Numb(1)));
		return new AssignWithBop(Plus, var, new Numb(1));
	case DECREMENT:
		Symb = readLexem();
		//return new Assign(var, new Bop(Minus, VarOrConst(id, offset, env), new Numb(1)));
		return new AssignWithBop(Minus, var, new Numb(1));
	default:
		ChybaExpanze("Assignment", Symb.type);
		return NULL;
	}
}

Expr * ConstructorCall(Env env) {
	ClassEnv * ce;
	char id[MAX_IDENT_LEN];

	Srovnani_IDENT(id);
	ce = hledejClass(id);
	if (!ce) {
		Chyba("Neexistujici konstruktor.");
	}

	Srovnani(LPAR);
	ArgList * a = Args(env);
	Srovnani(RPAR);

	return new New(new MethodRef(id), a);
}

Statm * AssignmentOrCall(Env env) {
	//char id[MAX_IDENT_LEN];

	Expr * e = Ident(env, false);
	Var * var = dynamic_cast<Var*>(e); // var
	if (!var) {
		return dynamic_cast<Statm*>(e); // call
	}
	return Assignment(env, var);
}

Statm * Prikaz(Env env, Context ctxt) {
	Var * var;
	skipNewlines();

	switch (Symb.type) {
	case kwVAR:
	case kwCONST:
		return Dekl(env, false);
	case kwRETURN:
		Symb = readLexem();
		return new Return(Vyraz(env));
	case kwTHIS:
	case IDENT: {
		return AssignmentOrCall(env);
	}
	case kwWRITE:
		Symb = readLexem();
		return new Write(Vyraz(env));
	case kwREAD:
		Symb = readLexem();
		var = dynamic_cast<Var*>(Ident(env, false)); // var
		if (!var) {
			Chyba("Ocekava se promenna.");
		}
		return new Read(var);
	case kwIF: {
		Symb = readLexem();
		Srovnani(LPAR);
		Expr *cond = Podminka(env);
		Srovnani(RPAR);
		Statm *prikaz = Prikaz(env, ctxt);
		return new If(cond, prikaz, CastElse(env, ctxt));
	}
	case kwWHILE: {
		Expr *cond;
		Symb = readLexem();
		Srovnani(LPAR);
		cond = Podminka(env);
		Srovnani(RPAR);
		return new While(cond, Prikaz(env, C_CYCLE));
	}
	case kwFOR: {
		//char id[MAX_IDENT_LEN];
		Symb = readLexem();
		Srovnani(LPAR);
		/*Srovnani_IDENT(id);
		Expr * offset = ArrayOffset(env, id);
		Var *var = new Var(adrProm(id, env.clsEnv, env.mthEnv), offset, false);*/
		Var * var = dynamic_cast<Var*>(Ident(env, false));
		if (!var) {
			Chyba("Ocekava se promenna.");
		}
		Srovnani(ASSIGN);
		Statm * init = new Assign(var, Vyraz(env));
		Expr * cond;
		Statm * counter;
		Statm * body;
		//ZbFor(env, id, offset, &cond, &counter, &body);
		ZbFor(env, NULL, NULL, &cond, &counter, &body);
		return new For(init, cond, counter, body);
	}
	case kwBREAK: {
		Symb = readLexem();
		if(ctxt == C_CYCLE){
			return new Break();
		}
		Chyba("Prikaz break je platny pouze uvnitr cyklu.");
		return new Empty;
	}

	case kwSWITCH: {
		Srovnani(kwSWITCH);
		Srovnani(LPAR);
		Expr *expr = Vyraz(env);
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

Expr * Podminka(Env env) {
	/*Expr *left = Vyraz();
	Operator op = RelOp();
	Expr *right = Vyraz();
	return new Bop(op, left, right);*/
	return Vyraz(env);
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

Expr * Vyraz(Env env) {
	return ZbVyrazu(env, LOrTerm(env));
}

Expr * ZbVyrazu(Env env, Expr * du) {
	switch (Symb.type) {
	case LOG_OR:
		Symb = readLexem();
		return ZbVyrazu(env, new Bop(LogOr, du, LOrTerm(env)));
	default:
		return du;
	}
}

Expr * LOrTerm(Env env) {
	return ZbLOrTermu(env, XorTerm(env));
}

Expr * ZbLOrTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case XOR:
		Symb = readLexem();
		return ZbLOrTermu(env, new Bop(Xor, du, XorTerm(env)));
	default:
		return du;
	}
}

Expr * XorTerm(Env env) {
	return ZbXorTermu(env, LAndTerm(env));
}

Expr * ZbXorTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case LOG_AND:
		Symb = readLexem();
		return ZbXorTermu(env, new Bop(LogAnd, du, LAndTerm(env)));
	default:
		return du;
	}
}

Expr * LAndTerm(Env env) {
	return ZbLAndTermu(env, BOrTerm(env));
}

Expr * ZbLAndTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case BIT_OR:
		Symb = readLexem();
		return ZbLAndTermu(env, new Bop(BitOr, du, BOrTerm(env)));
	default:
		return du;
	}
}

Expr * BOrTerm(Env env) {
	return ZbBOrTermu(env, BAndTerm(env));
}

Expr * ZbBOrTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case BIT_AND:
		Symb = readLexem();
		return ZbBOrTermu(env, new Bop(BitAnd, du, BAndTerm(env)));
	default:
		return du;
	}
}

Expr * BAndTerm(Env env) {
	return ZbBAndTermu(env, RelOpTerm(env));
}

Expr * ZbBAndTermu(Env env, Expr * du) {
	Operator op = RelOp();
	if (op == Error) {
		return du;
	}
	return ZbBAndTermu(env, new Bop(op, du, RelOpTerm(env)));
}

Expr * RelOpTerm(Env env) {
	return ZbRelOpTermu(env, ShiftTerm(env));
}

Expr * ZbRelOpTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case LSHIFT:
		Symb = readLexem();
		return ZbRelOpTermu(env, new Bop(Lsh, du, ShiftTerm(env)));
	case RSHIFT:
		Symb = readLexem();
		return ZbRelOpTermu(env, new Bop(Rsh, du, ShiftTerm(env)));
	default:
		return du;
	}
}

Expr * ShiftTerm(Env env) {
	return ZbShiftTermu(env, Term(env));
}

Expr * ZbShiftTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case PLUS:
		Symb = readLexem();
		return ZbShiftTermu(env, new Bop(Plus, du, Term(env)));
	case MINUS:
		Symb = readLexem();
		return ZbShiftTermu(env, new Bop(Minus, du, Term(env)));
	default:
		return du;
	}
}

Expr * Term(Env env) {
	return ZbTermu(env, Faktor(env));
}

Expr * ZbTermu(Env env, Expr * du) {
	switch (Symb.type) {
	case TIMES:
		Symb = readLexem();
		return ZbTermu(env, new Bop(Times, du, Faktor(env)));
	case DIVIDE:
		Symb = readLexem();
		return ZbVyrazu(env, new Bop(Divide, du, Faktor(env)));
	case MODULO:
		Symb = readLexem();
		return ZbVyrazu(env, new Bop(Modulo, du, Faktor(env)));
	default:
		return du;
	}
}

Expr * Faktor(Env env) {
	Expr * offset;

	if (Symb.type == MINUS) {
		Symb = readLexem();
		return new UnMinus(Faktor(env));
	}
	if (Symb.type == LOG_NOT) {
		Symb = readLexem();
		return new Not(Faktor(env));
	}

	switch (Symb.type) {
	case IDENT:
	case kwTHIS:
		//char id[MAX_IDENT_LEN];
		/*Srovnani_IDENT(id);
		offset = ArrayOffset(env, id);
		return VarOrConst(id, offset, env);*/
		return Ident(env, true);
	case NUMB:
		int hodn;
		Srovnani_NUMB(&hodn);
		return new Numb(hodn);
	case kwNEW:
		Symb = readLexem();
		return ConstructorCall(env);
	case LPAR: {
		Symb = readLexem();
		Expr *su = Vyraz(env);
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
