/* strom.h */

#ifndef _STROM_
#define _STROM_

#include "zaspoc.h"

class Node {
public:
	virtual Node *Optimize() {
		return this;
	}
	virtual void Translate() = 0;
	virtual ~Node() {
	}
	virtual void Print(int) = 0;
};

class Expr: public Node {
};

class Statm: public Node {
};

class Var: public Expr {
	int addr;
	Expr * offset;
	bool rvalue;
public:
	Var(int, Expr *, bool);
	virtual ~Var();
	virtual void Translate();
	virtual Node *Optimize();
	virtual void Print(int);
};

class Numb: public Expr {
	int value;
public:
	Numb(int);
	virtual void Translate();
	int Value();
	virtual void Print(int);
};

class Bop: public Expr {
	Operator op;
	Expr *left, *right;
public:
	Bop(Operator, Expr*, Expr*);
	virtual ~Bop();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class UnMinus: public Expr {
	Expr *expr;
public:
	UnMinus(Expr *e);
	virtual ~UnMinus();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Not: public Expr {
	Expr *expr;
public:
	Not(Expr *e);
	virtual ~Not();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Assign: public Statm {
	Var *var;
	Expr *expr;
public:
	Assign(Var*, Expr*);
	virtual ~Assign();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Write: public Statm {
	Expr *expr;
public:
	Write(Expr*);
	virtual ~Write();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Read : public Statm {
   Var * var;
public:
	Read(Var *);
	virtual ~Read();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class If: public Statm {
	Expr *cond;
	Statm *thenstm;
	Statm *elsestm;
public:
	If(Expr*, Statm*, Statm*);
	virtual ~If();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class While: public Statm {
	Expr *cond;
	Statm *body;
public:
	While(Expr*, Statm*);
	virtual ~While();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class For : public Statm {
   Statm *init;
   Expr *cond;
   Statm *counter;
   Statm *body;
public:
   For(Statm*, Expr*, Statm*, Statm*);
   virtual ~For();
   virtual Node *Optimize();
   virtual void Translate();
   virtual void Print(int);
};

class StatmList: public Statm {
	Statm *statm;
	StatmList *next;
public:
	StatmList(Statm*, StatmList*);
	virtual ~StatmList();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Break: public Statm {
public:
	virtual ~Break(){}
	virtual void Translate();
	virtual void Print(int);
};

class Empty: public Statm {
	virtual void Translate() {
	}
	virtual void Print(int);
};

class Class: public Node {
	StatmList *stm;
public:
	Class(StatmList*);
	virtual ~Class();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class ClassList: public Node {
	Class * cls;
	ClassList *next;
public:
	ClassList(Class*, ClassList*);
	virtual ~ClassList();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

class Prog: public Node {
	ClassList *lst;
public:
	Prog(ClassList*);
	virtual ~Prog();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

Expr *VarOrConst(char*, Expr * offset);

/* <nesro> */

/* this could/should be divided into three classes,
 * but this is PJP, not PA2 */
class CaseBlockScope {
public:
	Numb * lo;
	Numb * eq;
	Numb * hi;
	int type; /* 0: number, 1: range, 2: else */
	CaseBlockScope *next;
	CaseBlockScope(CaseBlockScope *);
	CaseBlockScope(CaseBlockScope *, Numb *);
	CaseBlockScope(CaseBlockScope *, Numb *, Numb *);
	~CaseBlockScope();
};

class CaseBlock: public Statm {
public:
	Statm *statmList;
	CaseBlockScope *scope;
	CaseBlock *next;
	CaseBlock();
	CaseBlock(Statm *, CaseBlock *, CaseBlockScope *);
	~CaseBlock();
	virtual void Translate();
	virtual void Print(int);
};

class Case: public Statm {
public:
	Expr *expr;
	CaseBlock *caseBlock;
	Case(Expr *, CaseBlock *);
	virtual ~Case();
	virtual Node *Optimize();
	virtual void Translate();
	virtual void Print(int);
};

#endif
