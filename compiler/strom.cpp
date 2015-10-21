/* strom.cpp */

#include "strom.h"
#include "tabsym.h"
#include <stdlib.h>

// konstruktory a destruktory

Var::Var(int a, Expr * o, bool rv) {
	addr = a;
	offset = o;
	rvalue = rv;
}

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

Assign::Assign(Var *v, Expr *e) {
	var = v;
	expr = e;
}

Assign::~Assign() {
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

Prog::Prog(StatmList *s) {
	stm = s;
}

Prog::~Prog() {
	delete stm;
}

// definice metody Optimize

Node *Var::Optimize() {
	if(offset){
		offset = (Expr*)(offset->Optimize());
	}
	return this;
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
	case Error: //cannot happen
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

Node *Assign::Optimize() {
	expr = (Var*) (expr->Optimize());
	return this;
}

Node *Write::Optimize() {
	expr = (Expr*) (expr->Optimize());
	return this;
}

Node *Read::Optimize() {
	var->Optimize();
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
	stm = (StatmList*) (stm->Optimize());
	return this;
}

// definice metody Translate

void Var::Translate() {
	Gener(TA, addr);
	if(offset){
		offset->Translate();
		Gener(BOP, Plus);
	}

	if (rvalue)
		Gener(DR);
}

void Numb::Translate() {
	Gener(TC, value);
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

void Assign::Translate() {
	var->Translate();
	expr->Translate();
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
	int a1 = Gener(IFJ);
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
	int a2 = Gener(IFJ);
	body->Translate();
	Gener(JU, a1);
	PutIC(a2);
}

void For::Translate() {
	init->Translate();
	int a1 = GetIC();
	cond->Translate();
	int a2 = Gener(IFJ);
	body->Translate();
	counter->Translate();
	Gener(JU, a1);
	PutIC(a2);
}

void StatmList::Translate() {
	StatmList *s = this;
	do {
		s->statm->Translate();
		s = s->next;
	} while (s);
}

void Prog::Translate() {
	stm->Translate();
	Gener(STOP);
}

Expr *VarOrConst(char *id, Expr * offset)
{
   int v;
   DruhId druh = idPromKonst(id,&v);
   switch (druh) {
   case IdProm:
      return new Var(v, offset, true);
   case IdKonst:
      return new Numb(v);
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
				jumpBeforeBlock[jbbp++] = Gener(IFJ);

				break;
			}
			case 2: /* range */{
				/* I __DO NOT__ need a negation of >= */
				Expr *rangeLo = new Bop(GreaterOrEq, this->expr,
						caseBlockScope->lo);
				rangeLo->Translate();

				/* if test of low rage fails, jump over the test of high range */
				int loRangeFail = Gener(IFJ);

				/* I need a negation of <= */
				Expr *rangeHi = new Bop(Greater, this->expr,
						caseBlockScope->hi);
				rangeHi->Translate();

				jumpBeforeBlock[jbbp++] = Gener(IFJ);

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
