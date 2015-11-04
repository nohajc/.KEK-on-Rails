/* strom.cpp */

#include "strom.h"
#include "tabsym.h"
#include "bcout.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// konstruktory a destruktory

Var::Var(const PrvekTab * sym, Expr * o, bool rv) {
	addr = sym->hodn;
	offset = o;
	rvalue = rv;
	sc = sym->sc;
	name = new char[strlen(sym->ident) + 1];
	strcpy(name, sym->ident);
}

Var::Var(char * n, bool rv) {
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	rvalue = rv;
}

Var::~Var() {
	delete offset;
	delete [] name;
}

ArgList::ArgList(Expr * a, ArgList * n) {
	arg = a;
	next = n;
}

ArgList::~ArgList() {
	delete arg;
	delete next;
}

Call::Call(Expr * m, ArgList * a) {
	method = m;
	args = a;
}

Call::~Call() {
	delete method;
	delete args;
}

New::New(MethodRef * c, ArgList * a) {
	constructor = c;
	args = a;
}

New::~New() {
	delete constructor;
}

ClassRef::ClassRef(char * n, bool rv, Expr * t) : Var(n, rv){
	target = t;
}

ClassRef::~ClassRef() {
	delete target;
}

ObjRef::ObjRef(const PrvekTab * sym, Expr * o, bool rv, Expr * t) : Var(sym, o, rv) {
	target = t;
}

ObjRef::~ObjRef() {
	delete target;
}

SelfRef::SelfRef(bool rv) : Var("this", rv) {}
ParentRef::ParentRef(bool rv, Expr * t) : Var("super", rv) {
	target = t;
}

MethodRef::MethodRef(char * n) {
	name = new char[strlen(n) + 1];
	strcpy(name, n);
}

MethodRef::~MethodRef() {
	delete [] name;
}

/*Var::Var(const char * n, int a, Expr * o, bool rv) {
	addr = a;
	offset = o;
	rvalue = rv;
	name = new char[strlen(n) + 1];
	strcpy(name, n);
}*/

Numb::Numb(int v) {
	value = v;
}

int Numb::Value() {
	return value;
}

Bop::Bop(Operator o, Expr *l, Expr *r) {
	op = o;
	left = l;
	right = r;
}

Bop::~Bop() {
	delete left;
	delete right;
}

UnMinus::UnMinus(Expr *e) {
	expr = e;
}

UnMinus::~UnMinus() {
	delete expr;
}

Not::Not(Expr *e) {
	expr = e;
}

Not::~Not(){
	delete expr;
}

Assign::Assign(Var *v, Expr *e) {
	var = v;
	expr = e;
}

Assign::~Assign() {
	delete var;
	delete expr;
}

AssignWithBop::AssignWithBop(Operator o, Var * v, Expr * e) {
	op = o;
	var = v;
	expr = e;
}

AssignWithBop::~AssignWithBop() {
	delete var;
	delete expr;
}

Write::Write(Expr *e) {
	expr = e;
}

Write::~Write() {
	delete expr;
}

Read::Read(Var * a) {
	var = a;
}

Read::~Read() {
}

If::If(Expr *c, Statm *ts, Statm *es) {
	cond = c;
	thenstm = ts;
	elsestm = es;
}

If::~If() {
	delete cond;
	delete thenstm;
	delete elsestm;
}

While::While(Expr *c, Statm *b) {
	cond = c;
	body = b;
}

While::~While() {
	delete cond;
	delete body;
}

For::For(Statm *i, Expr *c, Statm *cnt, Statm *b) {
	init = i;
	cond = c;
	counter = cnt;
	body = b;
}

For::~For() {
	delete init;
	delete cond;
	delete counter;
	delete body;
}

StatmList::StatmList(Statm *s, StatmList *n) {
	statm = s;
	next = n;
}

StatmList::~StatmList() {
	delete statm;
	delete next;
}

Prog::Prog(ClassList *l) {
	lst = l;
}

Prog::~Prog() {
	delete lst;
	symCleanup();
}

Class::Class(StatmList * s) {
	stm = s;
}

Class::~Class() {
	delete stm;
}

ClassList::ClassList(Class * c, ClassList * n) {
	cls = c;
	next = n;
}

ClassList::~ClassList() {
	delete cls;
	delete next;
}

Method::Method(char * n, bool sttc, int nArgs, unsigned int * bc_ep, StatmList * b) {
	name = new char[strlen(n) + 1];
	isStatic = sttc;
	numArgs = nArgs;
	body = b;
	bc_entrypoint = bc_ep;
	strcpy(name, n);
}

Method::~Method() {
	delete body;
	delete [] name;
}

Return::Return(Expr * e) {
	expr = e;
}

Return::~Return() {
	delete expr;
}

// definice metody Optimize

Node *Var::Optimize() {
	if(offset){
		offset = (Expr*)(offset->Optimize());
	}
	return this;
}

Node * ClassRef::Optimize() {
	target = (Expr*)(target->Optimize());
	return this;
}

Node * ObjRef::Optimize() {
	target = (Var*)(target->Optimize());
	return Var::Optimize();
}

Node *Bop::Optimize() {
	Numb *l = dynamic_cast<Numb*>(left->Optimize());
	Numb *r = dynamic_cast<Numb*>(right->Optimize());
	if (l) left = l;
	if (r) right = r;
	if (!l || !r)
		return this;
	int res;
	int leftval = l->Value();
	int rightval = r->Value();
	switch (op) {
	case Plus:
		res = leftval + rightval;
		break;
	case Minus:
		res = leftval - rightval;
		break;
	case Times:
		res = leftval * rightval;
		break;
	case Divide:
		res = leftval / rightval;
		break;
	case Modulo:
		res = leftval % rightval;
		break;
	case Eq:
		res = leftval == rightval;
		break;
	case NotEq:
		res = leftval != rightval;
		break;
	case Less:
		res = leftval < rightval;
		break;
	case Greater:
		res = leftval > rightval;
		break;
	case LessOrEq:
		res = leftval <= rightval;
		break;
	case GreaterOrEq:
		res = leftval >= rightval;
		break;
	case LogOr:
		res = leftval || rightval;
		break;
	case LogAnd:
		res = leftval && rightval;
		break;
	case BitOr:
		res = leftval | rightval;
		break;
	case BitAnd:
		res = leftval & rightval;
		break;
	case Xor:
		res = leftval ^ rightval;
		break;
	case Lsh:
		res = leftval << rightval;
		break;
	case Rsh:
		res = leftval >> rightval;
		break;
	case Error: //cannot happen
	default:
		abort();
		break;
	}
	delete this;
	return new Numb(res);
}

Node *UnMinus::Optimize() {
	expr->Optimize();
	Numb *e = dynamic_cast<Numb*>(expr);
	if (!e)
		return this;
	e = new Numb(-e->Value());
	delete this;
	return e;
}

Node *Not::Optimize() {
	expr->Optimize();
	Numb *e = dynamic_cast<Numb*>(expr);
	if (!e)
		return this;
	e = new Numb(!e->Value());
	delete this;
	return e;
}

Node *Assign::Optimize() {
	expr = (Expr*) (expr->Optimize());
	return this;
}

Node * AssignWithBop::Optimize() {
	expr = (Expr*) (expr->Optimize());
	return this;
}

Node *Write::Optimize() {
	expr = (Expr*) (expr->Optimize());
	return this;
}

Node *Read::Optimize() {
	var = (Var*) var->Optimize();
	return this;
}

Node *If::Optimize() {
	cond = (Expr*) (cond->Optimize());
	thenstm = (Statm*) (thenstm->Optimize());
	if(elsestm){
		elsestm = (Statm*)(elsestm->Optimize());
	}
	Numb *c = dynamic_cast<Numb*>(cond);
	if (!c)
		return this;
	Node *res;
	if (c->Value()) {
		res = thenstm;
		thenstm = 0;
	} else {
		res = elsestm;
		elsestm = 0;
	}
	delete this;
	return res;
}

Node *While::Optimize() {
	cond = (Expr*) (cond->Optimize());
	body = (Statm*) (body->Optimize());
	Numb *c = dynamic_cast<Numb*>(cond);
	if (!c)
		return this;
	if (!c->Value()) {
		delete this;
		return new Empty;
	}
	return this;
}

Node *For::Optimize() {
	cond = (Expr*)(cond->Optimize());
	body = (Statm*)(body->Optimize());
	Numb *c = dynamic_cast<Numb*>(cond);
	if (!c) return this;
	if (!c->Value()) {
		delete this;
		return new Empty;
	}
	return this;
}

Node *StatmList::Optimize() {
	StatmList *s = this;
	do {
		s->statm = (Statm*) (s->statm->Optimize());
		s = s->next;
	} while (s);

	return this;
}

Node *Prog::Optimize() {
	lst = (ClassList*) (lst->Optimize());
	return this;
}

Node *Class::Optimize() {
	if (stm) {
		stm = (StatmList*) (stm->Optimize());
	}
	return this;
}

Node *ClassList::Optimize() {
	ClassList *lst = this;
	do {
		lst->cls = (Class*) (lst->cls->Optimize());
		lst = lst->next;
	} while (lst);

	return this;
}

// definice metody Translate

void Var::Translate() { // TODO
	Gener(LDC, addr);
	if(offset){
		offset->Translate();
		Gener(BOP, Plus);
	}

	if (rvalue)
		Gener(LD);
}

void ClassRef::Translate() {
	target->Translate();
	 // TODO
}

void ObjRef::Translate() {
	Var::Translate();
	target->Translate();
	 // TODO
}

void Numb::Translate() {
	Gener(LDC, value);
	// TODO: create int object in constant table
	// Gener PUSH_C
}

void Bop::Translate() {
	left->Translate();
	right->Translate();
	Gener(BOP, op);
}

void UnMinus::Translate() {
	expr->Translate();
	Gener(UNM);
}

void Not::Translate() {
	expr->Translate();
	Gener(NOT);
}

void Assign::Translate() {
	var->Translate();
	expr->Translate();
	Gener(ST);
}

void AssignWithBop::Translate() {
	var->Translate();
	Gener(DUP);
	Gener(LD);
	expr->Translate();
	Gener(BOP, op);
	Gener(ST);
}

void Write::Translate() {
	expr->Translate();
	Gener(WRT);
}

void Read::Translate() {
	var->Translate();
	Gener(RD);
}

void If::Translate() {
	cond->Translate();
	int a1 = Gener(IFNJ);
	thenstm->Translate();
	if (elsestm) {
		int a2 = Gener(JU);
		PutIC(a1);
		elsestm->Translate();
		PutIC(a2);
	} else
		PutIC(a1);
}

void While::Translate() {
	int a1 = GetIC();
	cond->Translate();
	int a2 = Gener(IFNJ);

	int b1 = GetIC();
	body->Translate();
	int b2 = GetIC();
	
	Gener(JU, a1);
	resolveBreak(b1, b2);
	PutIC(a2);
}

void For::Translate() {
	init->Translate();
	int a1 = GetIC();
	cond->Translate();
	int a2 = Gener(IFNJ);

	int b1 = GetIC();
	body->Translate();
	int b2 = GetIC();

	counter->Translate();

	Gener(JU, a1);
	resolveBreak(b1, b2);
	PutIC(a2);
}

void StatmList::Translate() {
	StatmList *s = this;
	do {
		s->statm->Translate();
		s = s->next;
	} while (s);
}

void Break::Translate() {
	Gener(JU, 0, UBRK);
}

void Prog::Translate() {
	lst->Translate();
	Gener(STOP);
}

void Class::Translate() {
	if (stm) {
		stm->Translate();
	}
}

void ClassList::Translate() {
	ClassList * lst = this;
	do {
		lst->cls->Translate();
		lst = lst->next;
	} while (lst);
}

void Method::Translate() {
	 // TODO
	body->Translate();
}

void Return::Translate() {
	expr->Translate();
	// TODO: implement
}

Expr *VarOrConst(char *id, Expr * offset, Env env)
{
   int v;
   PrvekTab * p = adrSym(id, env.clsEnv, env.mthEnv);
   //DruhId druh = idPromKonst(id, &v, env.clsEnv, env.mthEnv);
   DruhId druh = p->druh;

   switch (druh) {
   case IdProm:
      return new Var(p, offset, true);
   case IdKonst:
      return new Numb(p->hodn);
   default:
      return 0;
   }
}

CaseBlockScope::CaseBlockScope(CaseBlockScope *next_) {
	this->lo = NULL;
	this->eq = NULL;
	this->hi = NULL;
	this->type = 0;
	this->next = next_;
}

CaseBlockScope::CaseBlockScope(CaseBlockScope *next_, Numb *eq_) {
	this->lo = NULL;
	this->eq = eq_;
	this->hi = NULL;
	this->type = 1;
	this->next = next_;
}

CaseBlockScope::CaseBlockScope(CaseBlockScope *next_, Numb *lo_, Numb *hi_) {
	this->lo = lo_;
	this->eq = NULL;
	this->hi = hi_;
	this->type = 2;
	this->next = next_;
}

CaseBlockScope::~CaseBlockScope() {
	delete this->next;
	delete this->lo;
	delete this->eq;
	delete this->hi;
}

CaseBlock::CaseBlock() {
	this->statmList = NULL;
	this->next = NULL;
	this->scope = NULL;
}

CaseBlock::CaseBlock(Statm *statmList_, CaseBlock *next_,
		CaseBlockScope *scope_) {
	this->statmList = statmList_;
	this->next = next_;
	this->scope = scope_;
}

CaseBlock::~CaseBlock() {
	delete this->statmList;
	delete this->next;
	delete this->scope;
}

void CaseBlock::Translate() {
	if (this->statmList != NULL) {
		this->statmList->Translate();
	} else {
	}
}

Case::Case(Expr *expr_, CaseBlock *caseBlock_) {
	this->expr = expr_;
	this->caseBlock = caseBlock_;
}

Case::~Case() {
	delete this->expr;
	delete this->caseBlock;
}

Node * Case::Optimize() {
	this->expr = (Expr *) (this->expr->Optimize());

	/* Check, if the Expr is just a number. */
	Numb *numbTest = dynamic_cast<Numb*>(this->expr);

	/* If we have a number, we could decide which scope will match! */
	if (numbTest != NULL) {
		/* Run through all the blocks. */
		for (CaseBlock *caseBlock_p = this->caseBlock; caseBlock_p != NULL;
				caseBlock_p = caseBlock_p->next) {

			/* Run through all the scopes. */
			for (CaseBlockScope *caseBlockScope_p = caseBlock_p->scope;
					caseBlockScope_p != NULL; caseBlockScope_p =
							caseBlockScope_p->next) {

				switch (caseBlockScope_p->type) {
				case 0: /* else */
					caseBlock_p->statmList =
							(Statm *) (caseBlock_p->statmList->Optimize());

					return caseBlock_p;
				case 1: /* number */
					if (numbTest->Value() == caseBlockScope_p->eq->Value()) {
						caseBlock_p->statmList =
								(Statm *) (caseBlock_p->statmList->Optimize());

						return caseBlock_p;
					}

					break;
				case 2: /* range */
					if (numbTest->Value() >= caseBlockScope_p->lo->Value()
							&& numbTest->Value()
									<= caseBlockScope_p->hi->Value()) {
						caseBlock_p->statmList =
								(Statm *) (caseBlock_p->statmList->Optimize());

						return caseBlock_p;
					}

					break;
				default:
					break;
				}
			}
		}

		/* No scope hasn't been matched by nubmer. */
		delete this;
		return new Empty;
	}

	/* Delete all nonsense ranges (e.g. 4..2, etc.). Replace ranges matching
	 * one number witch just a number (e.g. 2..2 convert to 2). Optimize
	 * statms. */

	/* Run through all the blocks. */
	for (CaseBlock *caseBlock_p = this->caseBlock; caseBlock_p != NULL;
			caseBlock_p = caseBlock_p->next) {

		/* Else block. */
		if (caseBlock_p->statmList == NULL) {
			continue;
		}

		caseBlock_p->statmList = (Statm *) (caseBlock_p->statmList->Optimize());

		/* Run through all the scopes. */
		for (CaseBlockScope *caseBlockScope_p = caseBlock_p->scope,
				*caseBlockScope_pBack = NULL; caseBlockScope_p != NULL;
				caseBlockScope_pBack = caseBlockScope_p, caseBlockScope_p =
						caseBlockScope_p->next) {

			if (caseBlockScope_p->type == 2) { /* range */

				if (caseBlockScope_p->lo->Value()
						== caseBlockScope_p->hi->Value()) {
					caseBlockScope_p->type = 1; /* number */
					caseBlockScope_p->eq = caseBlockScope_p->lo;
				} else if (caseBlockScope_p->lo->Value()
						> caseBlockScope_p->hi->Value()) {

					if (caseBlockScope_pBack != NULL) {
						caseBlockScope_pBack->next = caseBlockScope_p->next;
						caseBlockScope_p = caseBlockScope_pBack;
					} else {
						caseBlock_p->scope = caseBlockScope_p->next;
					}

					/* FIXME: memory leak! */
				}
			}
		}
	}

	return this;
}

void Case::Translate() {
	int finalJumps[1000];
	int fjp = 0;

	int jumpBeforeBlock[1000];
	int jbbp = 0;

	for (CaseBlock *nextBlock = this->caseBlock; nextBlock != NULL; nextBlock =
			nextBlock->next) {

		for (CaseBlockScope *caseBlockScope = nextBlock->scope;
				caseBlockScope != NULL; caseBlockScope = caseBlockScope->next) {

			switch (caseBlockScope->type) {
			case 0: /* else*/{
				/* just jump, else don't need any BOP */
				jumpBeforeBlock[jbbp++] = Gener(JU);
				break;
			}
			case 1: /* number */{
				/* I need a negation of == */
				Expr *rangeEq = new Bop(NotEq, this->expr, caseBlockScope->eq);
				rangeEq->Translate();

				/* if the bop made false from negation, so true from what we
				 * need, jump before block! */
				jumpBeforeBlock[jbbp++] = Gener(IFNJ);

				break;
			}
			case 2: /* range */{
				/* I __DO NOT__ need a negation of >= */
				Expr *rangeLo = new Bop(GreaterOrEq, this->expr,
						caseBlockScope->lo);
				rangeLo->Translate();

				/* if test of low rage fails, jump over the test of high range */
				int loRangeFail = Gener(IFNJ);

				/* I need a negation of <= */
				Expr *rangeHi = new Bop(Greater, this->expr,
						caseBlockScope->hi);
				rangeHi->Translate();

				jumpBeforeBlock[jbbp++] = Gener(IFNJ);

				/* */
				PutIC(loRangeFail);

				break;
			}
			default:
				break;
			}

		}

		/* if we get here, we have not matched any scope! */
		int jumpOver = Gener(JU);

		/* if something matches, jump before the block */
		for (int i = 0; i < jbbp; i = i + 1) {
			PutIC(jumpBeforeBlock[i]);
		}
		jbbp = 0;

		/* if all case block has matched, run the block! */
		nextBlock->Translate();

		/* if we have matched and block was run, jump to the end of the case */
		if (nextBlock->next != NULL) {
			finalJumps[fjp++] = Gener(JU);
		}

		/*  */
		PutIC(jumpOver);
	}

	/* after every matched scope, hop/skip/jump to the end of the case */
	for (int i = 0; i < fjp; i = i + 1) {
		PutIC(finalJumps[i]);
	}
}

/******************************************************************************/
/* printing *******************************************************************/

void printfi(int ident, const char *t, ...) {
	va_list ap;
	int i;

	for (i = 0; i < ident; i++) {
			printf("|----");
	}

	va_start(ap, t);
	vfprintf(stdout, t, ap);
	va_end(ap);
}

void Var::Print(int ident) {
	printfi(ident, "Var [addr=%d]\n", this->addr);

	printfi(ident, "offset:\n");
	if (this->offset) {
		this->offset->Print(ident + 1);
	}
}

void ArgList::Print(int ident) {
	printfi(ident, "ArgList\n");

	ArgList * a = this;
	do {
		printfi(ident, "arg:\n");
		if (a->arg) {
			a->arg->Print(ident + 1);
		}
		a = a->next;
	} while (a);
}

void Call::Print(int ident) {
	printfi(ident, "Call\n");

	printfi(ident, "method:\n");
	if (this->method) {
		this->method->Print(ident + 1);
	}

	printfi(ident, "args:\n");
	if (this->args) {
		this->args->Print(ident + 1);
	}
}

void ClassRef::Print(int ident) {
	printfi(ident, "ClassRef\n");

	printfi(ident, "target:\n");
	if (this->target) {
		target->Print(ident + 1);
	}
}

void ObjRef::Print(int ident) {
	printfi(ident, "ObjRef\n");

	printfi(ident, "target:\n");
	if (this->target) {
		target->Print(ident + 1);
	}
}

void MethodRef::Print(int ident) {
	printfi(ident, "MethodRef\n");

	printfi(ident, "name: %s\n", name);
}

void SelfRef::Print(int ident) {
	printfi(ident, "SelfRef\n");
}

void ParentRef::Print(int ident) {
	printfi(ident, "ParentRef\n");

	printfi(ident, "target:\n");
	if (this->target) {
		target->Print(ident + 1);
	}
}

void New::Print(int ident) {
	printfi(ident, "New\n");
	printfi(ident, "constructor:\n");
	constructor->Print(ident + 1);

	printfi(ident, "args:\n");
	if (args) {
		args->Print(ident + 1);
	}
}

void Numb::Print(int ident) {
	printfi(ident, "Numb [value=%d]\n", this->value);
}

void Bop::Print(int ident) {
	printfi(ident, "Bop\n");

	printfi(ident, "left:\n");
	if (this->left) {
		this->left->Print(ident + 1);
	}

	printfi(ident, "right:\n");
	if (this->right) {
		this->right->Print(ident + 1);
	}
}

void UnMinus::Print(int ident) {
	printfi(ident, "UnMinus\n");

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void Not::Print(int ident) {
	printfi(ident, "Not\n");

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void Assign::Print(int ident) {
	printfi(ident, "Assign\n");

	printfi(ident, "var:\n");
	if (this->var) {
		this->var->Print(ident + 1);
	}

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void AssignWithBop::Print(int ident) {
	printfi(ident, "AssignWithBop\n");

	printfi(ident, "var:\n");
	if (this->var) {
		this->var->Print(ident + 1);
	}

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void Write::Print(int ident) {
	printfi(ident, "Write\n");

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void Read::Print(int ident) {
	printfi(ident, "Read\n");

	printfi(ident, "var:\n");
	if (this->var) {
		this->var->Print(ident + 1);
	}
}

void If::Print(int ident) {
	printfi(ident, "If\n");

	printfi(ident, "cond:\n");
	if (this->cond) {
		this->cond->Print(ident + 1);
	}

	printfi(ident, "thenstm:\n");
	if (this->thenstm) {
		this->thenstm->Print(ident + 1);
	}

	printfi(ident, "elsestm:\n");
	if (this->elsestm) {
		this->elsestm->Print(ident + 1);
	}
}

void While::Print(int ident) {
	printfi(ident, "While\n");

	printfi(ident, "cond:\n");
	if (this->cond) {
		this->cond->Print(ident + 1);
	}

	printfi(ident, "body:\n");
	if (this->body) {
		this->body->Print(ident + 1);
	}
}

void For::Print(int ident) {
	printfi(ident, "For\n");

	printfi(ident, "init:\n");
	if (this->init) {
		this->init->Print(ident + 1);
	}

	printfi(ident, "cond:\n");
	if (this->cond) {
		this->cond->Print(ident + 1);
	}

	printfi(ident, "counter:\n");
	if (this->counter) {
		this->counter->Print(ident + 1);
	}

	printfi(ident, "body:\n");
	if (this->body) {
		this->body->Print(ident + 1);
	}
}

void StatmList::Print(int ident) {
	printfi(ident, "StatmList\n");

	StatmList *s = this;
	do {
		printfi(ident, "statm:\n");
		if (s->statm) {
			s->statm->Print(ident + 1);
		}
		s = s->next;
	} while (s);
}

void Break::Print(int ident) {
	printfi(ident, "Break\n");
}

void Empty::Print(int ident) {
	printfi(ident, "Empty\n");
}

void Prog::Print(int ident) {
	printfi(ident, "Prog\n");

	printfi(ident, "lst:\n");
	if (this->lst) {
		this->lst->Print(ident + 1);
	}
}

void Class::Print(int ident) {
	printfi(ident, "Class\n");

	printfi(ident, "stm:\n");
	if (this->stm) {
		this->stm->Print(ident + 1);
	}
}

void ClassList::Print(int ident) {
	printfi(ident, "ClassList\n");

	ClassList * lst = this;
	do {
		printfi(ident, "cls:\n");
		if (lst->cls) {
			lst->cls->Print(ident + 1);
		}
		lst = lst->next;
	} while(lst);
}

void Method::Print(int ident) {
	printfi(ident, "Method\n");

	printfi(ident, "body:\n");
	if (this->body) {
		this->body->Print(ident + 1);
	}
}

void Return::Print(int ident) {
	printfi(ident, "Return\n");

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}
}

void CaseBlock::Print(int ident) {
	printfi(ident, "CaseBlock\n");

	printfi(ident, "statm:\n");
	if (this->statmList) {
		this->statmList->Print(ident + 1);
	}

	printfi(ident, "next:\n");
	if (this->next) {
		this->next->Print(ident + 1);
	}
}

void Case::Print(int ident) {
	printfi(ident, "CaseBlock\n");

	printfi(ident, "expr:\n");
	if (this->expr) {
		this->expr->Print(ident + 1);
	}

	printfi(ident, "caseBlock:\n");
	if (this->caseBlock) {
		this->caseBlock->Print(ident + 1);
	}
}
