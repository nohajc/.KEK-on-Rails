/* strom.h */

#ifndef _STROM_
#define _STROM_

#include "tabsym.h"
#include "bcout.h"
#include <stdint.h>

class Node {
public:
	virtual Node *Optimize() {
		return this;
	}
	virtual uint32_t Translate() = 0;
	virtual ~Node() {
	}
	virtual void Print(int) = 0;
};

class Expr: public Node {
};

class Const: public Expr {
};

class Statm: public Expr {
};

class ArgList: public Expr {
public:
	Expr * arg;
	ArgList * next;
	ArgList(Expr *, ArgList *);
	virtual ~ArgList();
	virtual uint32_t Translate();
	virtual Node *Optimize();
	virtual void Print(int);
	int Count();
};

class Var: public Expr {
protected:
	char * name;
	int addr;
	ArgList * offset;
	bool rvalue;
	Scope sc;
	bool external;
public:
	Var(const PrvekTab * sym, ArgList * o, bool rv, bool e);
	Var(char * n, bool rv);
	Var(char * n, ArgList * o, bool rv, bool e);
	virtual ~Var();
	virtual uint32_t Translate();
	virtual Node *Optimize();
	virtual void Print(int);
};

class Call: public Statm {
	Expr * method;
	bool external;
	ArgList * args;
public:
	Call(Expr *, ArgList *, bool);
	virtual ~Call();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class ClassRef: public Var { // Var which is also a class
	Expr * target; // Member of the referenced class - must be static
public:
	ClassRef(char *, bool, Expr *);
	virtual ~ClassRef();
	virtual uint32_t Translate();
	virtual Node *Optimize();
	virtual void Print(int);
};

class ObjRef: public Var { // Var which is also an object
	Expr * target; // Member of the referenced object
public:
	ObjRef(const PrvekTab *, ArgList *, bool, bool, Expr *);
	virtual ~ObjRef();
	virtual uint32_t Translate();
	virtual Node *Optimize();
	virtual void Print(int);
};

class SelfRef: public Var {
public:
	SelfRef(bool);
	virtual uint32_t Translate();
	virtual void Print(int);
};

class ParentRef: public Var {
	Expr * target;
public:
	ParentRef(bool, Expr *);
	virtual uint32_t Translate();
	virtual void Print(int);
};

class MethodRef: public Expr {
	char * name;
public:
	MethodRef(char *);
	virtual ~MethodRef();
	virtual uint32_t Translate(); // returns index of the function name in constant table
	virtual void Print(int);
};

class New: public Expr {
	MethodRef * constructor;
	ArgList * args;
public:
	New(MethodRef *, ArgList *);
	virtual ~New();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Numb: public Const {
	int value;
public:
	Numb(int);
	virtual uint32_t Translate();
	int Value();
	virtual void Print(int);
};

class String: public Const {
	char * value;
public:
	String(const char *);
	virtual ~String();
	virtual uint32_t Translate();
	char * Value();
	virtual void Print(int);
};

class Array: public Const {
	ArgList * elems;
public:
	Array(ArgList *);
	virtual ~Array();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Nil: public Const {
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Bop: public Expr {
	Operator op;
	Expr *left, *right;
public:
	Bop(Operator, Expr*, Expr*);
	virtual ~Bop();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class UnMinus: public Expr {
	Expr *expr;
public:
	UnMinus(Expr *e);
	virtual ~UnMinus();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Not: public Expr {
	Expr *expr;
public:
	Not(Expr *e);
	virtual ~Not();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Assign: public Statm {
	Var *var;
	Expr *expr;
public:
	Assign(Var*, Expr*);
	virtual ~Assign();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class AssignWithBop: public Statm {
	Operator op;
	Var * var;
	Expr * expr;
public:
	AssignWithBop(Operator, Var *, Expr *);
	virtual ~AssignWithBop();
	virtual Node * Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Write: public Statm {
	Expr *expr;
public:
	Write(Expr*);
	virtual ~Write();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Read : public Statm {
   Var * var;
public:
	Read(Var *);
	virtual ~Read();
	virtual Node *Optimize();
	virtual uint32_t Translate();
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
	virtual uint32_t Translate();
	virtual void Print(int);
};

class While: public Statm {
	Expr *cond;
	Statm *body;
public:
	While(Expr*, Statm*);
	virtual ~While();
	virtual Node *Optimize();
	virtual uint32_t Translate();
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
   virtual uint32_t Translate();
   virtual void Print(int);
};

class StatmList: public Statm {
public:
	Statm *statm;
	StatmList *next;
	StatmList(Statm*, StatmList*);
	virtual ~StatmList();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Break: public Statm {
public:
	virtual ~Break(){}
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Empty: public Statm {
	virtual uint32_t Translate() { return 0; }
	virtual void Print(int);
};

class Class: public Statm {
	char * name;
	StatmList *stm;
public:
	Class(const char *, StatmList*);
	virtual ~Class();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class ClassList: public Statm {
	Class * cls;
	ClassList *next;
public:
	ClassList(Class*, ClassList*);
	virtual ~ClassList();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

/*class MethodList: public StatmList{
};*/

class Method: public Statm {
	char * name;
	bool isStatic;
	int numArgs;
	StatmList * body;
	unsigned int * bc_entrypoint; // Points to symbol table MethodEnv object
public:
	Method(const char *, bool, int, unsigned int *, StatmList *);
	virtual ~Method();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Return: public Statm {
	Expr * expr;
public:
	Return(Expr *);
	virtual ~Return();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Prog: public Node {
	ClassList *lst;
public:
	Prog(ClassList*);
	virtual ~Prog();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

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
	virtual ~CaseBlockScope();
	virtual void Print(int);
};

class CaseBlock: public Statm {
public:
	Statm *statmList;
	CaseBlockScope *scope;
	CaseBlock *next;
	CaseBlock();
	CaseBlock(Statm *, CaseBlock *, CaseBlockScope *);
	virtual ~CaseBlock();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

class Case: public Statm {
public:
	Expr *expr;
	CaseBlock *caseBlock;
	Case(Expr *, CaseBlock *);
	virtual ~Case();
	virtual Node *Optimize();
	virtual uint32_t Translate();
	virtual void Print(int);
};

#endif
