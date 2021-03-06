/* strom.cpp */

#include "strom.h"
#include "tabsym.h"
#include "bcout.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// konstruktory a destruktory

Var::Var(const PrvekTab * sym, ArgList * o, bool rv, bool e, bool uc) {
	addr = sym->addr;
	offset = o;
	rvalue = rv;
	sc = sym->sc;
	external = e;
	name = new char[strlen(sym->ident) + 1];
	strcpy(name, sym->ident);
	unknown_class = uc;
}

Var::Var(char * n, bool rv) {
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	rvalue = rv;
	offset = NULL;
	external = false;
	addr = -1;
}

Var::Var(char * n, Scope s, ArgList * o, bool rv, bool e, bool uc) {
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	rvalue = rv;
	offset = o;
	external = e;
	addr = -1;
	sc = s;
	unknown_class = uc;
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

Call::Call(Expr * m, ArgList * a, bool e, Expr * t, bool vc) {
	method = m;
	args = a;
	external = e;
	target = t;
	void_call = vc;
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

ObjRef::ObjRef(const PrvekTab * sym, ArgList * o, bool rv, bool e, bool uc, Expr * t) : Var(sym, o, rv, e, uc) {
	target = t;
}

ObjRef::ObjRef(char *n, Scope s, ArgList * o, bool rv, bool e, bool uc, Expr * t) : Var(n, s, o, rv, e, uc) {
	target = t;
}

ObjRef::~ObjRef() {
	delete target;
}

SelfRef::SelfRef(bool rv) : Var("this", rv) {}

ParentRef::ParentRef(bool rv, char * pn, Expr * t) : Var("super", rv) {
	parName = new char[strlen(pn) + 1];
	strcpy(parName, pn);
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

String::String(const char * v) {
	value = new char[strlen(v) + 1];
	strcpy(value, v);
}

String::~String() {
	delete [] value;
}

Symbol::Symbol(const char * v) {
	value = new char[strlen(v) + 1];
	strcpy(value, v);
}

Symbol::~Symbol() {
	delete [] value;
}

Array::Array(ArgList * e) {
	elems = e;
}

Array::~Array() {
	delete elems;
}

int Numb::Value() {
	return value;
}

char * String::Value() {
	return value;
}

char * Symbol::Value() {
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

Try::Try(Statm *tb, Statm *cb, MethodEnv *me) {
	try_block = tb;
	catch_block = cb;
	mthEnv = me;
	mthEnv->try_block_cnt++;
}

Try::~Try() {
	delete try_block;
	delete catch_block;
}

Throw::Throw(Expr *o) {
	obj = o;
}

Throw::~Throw() {
	delete obj;
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

Class::Class(const char * n, StatmList * s) {
	name = new char[strlen(n) + 1];
	strcpy(name, n);
	stm = s;
}

Class::~Class() {
	delete stm;
	delete [] name;
}

ClassList::ClassList(Class * c, ClassList * n) {
	cls = c;
	next = n;
}

ClassList::~ClassList() {
	delete cls;
	delete next;
}

Method::Method(const char * n, bool sttc, bool cons, int nArgs, MethodEnv * me, StatmList * b) {
	name = new char[strlen(n) + 1];
	isStatic = sttc;
	isConstructor = cons;
	numArgs = nArgs;
	body = b;
	mthEnv = me;
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

Node *ArgList::Optimize() {
	arg = (Expr*)arg->Optimize();
	if (next) {
		next = (ArgList*)next->Optimize();
	}
	return this;
}

Node *Var::Optimize() {
	if(offset){
		offset = (ArgList*)(offset->Optimize());
	}
	return this;
}

Node *Call::Optimize() {
	if (args) {
		args = (ArgList*)args->Optimize();
	}
	return this;
}

Node * ClassRef::Optimize() {
	Const * cnst_target;
	if (target) {
		target = (Expr*)(target->Optimize());
		cnst_target = dynamic_cast<Const*>(target);


		if (cnst_target) {
			target = NULL;
			delete this;
			return cnst_target;
		}
	}

	return this;
}

Node * ObjRef::Optimize() {
	target = (Expr*)(target->Optimize());
	return Var::Optimize();
}

Node * ParentRef::Optimize() {
	target = (Expr*)(target->Optimize());
	return Var::Optimize();
}

Node * New::Optimize() {
	if (args) {
		args = (ArgList*)args->Optimize();
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

Node *Try::Optimize() {
	try_block = (Statm*)(try_block->Optimize());
	catch_block = (Statm*)(catch_block->Optimize());
	return this;
}

Node *Throw::Optimize() {
	obj = (Expr*)(obj->Optimize());
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

Node *Method::Optimize() {
	body = (StatmList*)body->Optimize();
	return this;
}

Node *Return::Optimize() {
	expr = (Expr*)(expr->Optimize());
	return this;
}

// definice metody Translate

uint32_t Var::Translate() {
	switch (sc) {
	case SC_LOCAL:
		bco_ww1(bcout_g, (rvalue || offset ? LVBI_LOC : LABI_LOC), addr);
		break;
	case SC_ARG:
		bco_ww1(bcout_g, (rvalue || offset ? LVBI_ARG : LABI_ARG), addr);
		break;
	case SC_INSTANCE:
		if (external) {
			uint32_t iv_name_idx = bco_sym(bcout_g, name);
			bco_ww1(bcout_g, (rvalue || offset ? LVBS_IVE : LABS_IVE), iv_name_idx);
		}
		else {
			bco_ww1(bcout_g, (rvalue || offset ? LVBI_IV : LABI_IV), addr);
		}
		break;
	case SC_CLASS:
		if (external) {
			if (unknown_class) {
				uint32_t cve_name_idx = bco_sym(bcout_g, name);
				bco_ww1(bcout_g, (rvalue || offset ? LVBS_CVE : LABS_CVE), cve_name_idx);
			}
			else {
				bco_ww1(bcout_g, (rvalue || offset ? LVBI_CVE : LABI_CVE), addr);
			}
		}
		else {
			bco_ww1(bcout_g, (rvalue || offset ? LVBI_CV : LABI_CV), addr);
		}
		break;
	case SC_EXOBJ:
		bco_w0(bcout_g, LD_EXOBJ);
	}

	if (offset) {
		ArgList * o = offset;
		while (o->next) {
			o->arg->Translate();
			bco_w0(bcout_g, IDX);
			o = o->next;
		}

		o->arg->Translate();
		bco_w0(bcout_g, (rvalue ? IDX : IDXA));
	}


	/*if(offset){
		offset->Translate();
		bco_w0(bcout_g, (rvalue ? IDX : IDXA));
	}*/

	return 0;
}

uint32_t ArgList::Translate() {
	ArgList * a = this;
	do {
		a->arg->Translate();
		a = a->next;
	} while (a);

	return 0;
}

uint32_t Call::Translate() {
	uint32_t mth_idx;
	int arg_count = args ? args->Count() : 0;
	ParentRef * parRef;

	if (args) {
		args->Translate(); // Puts arguments on the stack
	}
	mth_idx = method->Translate(); // Puts classref, objref (for CALLE) or nothing on the stack

	if ((parRef = dynamic_cast<ParentRef*>(method))) {
		constant_string_t * mth_sym = (constant_string_t*) (bcout_g->const_table + mth_idx);
		if (mth_sym->h.cls) {
			constant_string_t * saved_parName = (constant_string_t*) (bcout_g->const_table + mth_sym->h.cls);
			if (strcmp(saved_parName->string, parRef->parName)) {
				mth_idx = bco_sym(bcout_g, mth_sym->string, true);
				mth_sym = (constant_string_t*)(bcout_g->const_table + mth_idx);
			}
		}

		mth_sym->h.cls = bco_sym(bcout_g, parRef->parName);

		bco_ww2(bcout_g, CALLS, (uint16_t)mth_idx, (uint16_t)arg_count);
	}
	else if (external) {
		bco_ww2(bcout_g, CALLE, (uint16_t)mth_idx, (uint16_t)arg_count);
	}
	else {
		bco_ww2(bcout_g, CALL, (uint16_t)mth_idx, (uint16_t)arg_count);
	}
	for (int i = 0; i < 16; i++) {
		bco_w0(bcout_g, NOP);
	}

	if (target) {
		target->Translate();
	}
	else if (void_call) {
		bco_w0(bcout_g, POP);
	}

	return 0;
}

uint32_t ClassRef::Translate() {
	uint32_t sym_idx = bco_sym(bcout_g, name);
	bco_ww1(bcout_g, LD_CLASS, (uint16_t)sym_idx);

	if (target) {
		return target->Translate();
	}
	return 0;
}

uint32_t ObjRef::Translate() {
	Var::Translate();
	return target->Translate();
}

uint32_t SelfRef::Translate() {
	bco_w0(bcout_g, LD_SELF);
	return 0;
}

uint32_t ParentRef::Translate() {
	return target->Translate();
}

uint32_t MethodRef::Translate() {
	return bco_sym(bcout_g, name);
}

uint32_t New::Translate() {
	int arg_count = args ? args->Count() : 0;
	if (args) {
		args->Translate();
	}
	uint32_t cons_idx = constructor->Translate();
	bco_ww2(bcout_g, NEW, cons_idx, arg_count);
	return 0;
}

uint32_t Numb::Translate() {
	uint32_t num_idx = bco_int(bcout_g, value);
	bco_ww1(bcout_g, LVBI_C, num_idx); // TODO: optimize?

	return 0;
}

uint32_t String::Translate() {
	uint32_t str_idx = bco_str(bcout_g, value);
	bco_ww1(bcout_g, LVBI_C, str_idx);

	return 0;
}

uint32_t Symbol::Translate() {
	uint32_t sym_idx = bco_sym(bcout_g, value);
	bco_ww1(bcout_g, LVBI_C, sym_idx);

	return 0;
}

uint32_t construct_const_array(Array & a) {
	ArgList * e = a.elems;
	Numb * n;
	String * str;
	Symbol * sym;
	Array * inner_a;
	int elem_count = e->Count();
	uint32_t arr_idx = bco_arr(bcout_g, elem_count);
	int i = 0;

	while (e) {
		if ((n = dynamic_cast<Numb*>(e->arg))) {
			uint32_t int_idx = bco_int(bcout_g, n->Value());
			bco_arr_set_idx(bcout_g, arr_idx, i, int_idx);
			//printf("DEBUG int_idx = %u\n", int_idx);
		}
		else if((str = dynamic_cast<String*>(e->arg))) {
			uint32_t str_idx = bco_str(bcout_g, str->Value());
			bco_arr_set_idx(bcout_g, arr_idx, i, str_idx);
			//printf("DEBUG str_idx = %u\n", str_idx);
		}
		else if((sym = dynamic_cast<Symbol*>(e->arg))) {
			uint32_t sym_idx = bco_sym(bcout_g, sym->Value());
			bco_arr_set_idx(bcout_g, arr_idx, i, sym_idx);
			//printf("DEBUG str_idx = %u\n", str_idx);
		}
		else if((inner_a = dynamic_cast<Array*>(e->arg))) {
			uint32_t inner_a_idx = construct_const_array(*inner_a);
			bco_arr_set_idx(bcout_g, arr_idx, i, inner_a_idx);
		}
		i++;

		e = e->next;
	}

	return arr_idx;
}

uint32_t Array::Translate() {
	uint32_t arr_idx = construct_const_array(*this);
	bco_ww1(bcout_g, LVBI_C, arr_idx);

	return 0;
}

uint32_t Nil::Translate() {
	uint32_t nil_idx = bco_nil(bcout_g);
	bco_ww1(bcout_g, LVBI_C, nil_idx);

	return 0;
}

uint32_t Bop::Translate() {
	left->Translate();
	right->Translate();
	bco_wb1(bcout_g, BOP, op);
	return 0;
}

uint32_t UnMinus::Translate() {
	expr->Translate();
	bco_w0(bcout_g, UNM);
	return 0;
}

uint32_t Not::Translate() {
	expr->Translate();
	bco_w0(bcout_g, NOT);
	return 0;
}

uint32_t Assign::Translate() {
	var->Translate();
	expr->Translate();
	bco_w0(bcout_g, ST);
	return 0;
}

uint32_t AssignWithBop::Translate() {
	var->Translate();
	bco_w0(bcout_g, DUP);
	bco_w0(bcout_g, DR);
	expr->Translate();
	bco_wb1(bcout_g, BOP, op);
	bco_w0(bcout_g, ST);
	return 0;
}

uint32_t Write::Translate() {
	expr->Translate();
	bco_w0(bcout_g, WRT);
	return 0;
}

uint32_t Read::Translate() {
	var->Translate();
	bco_w0(bcout_g, RD);
	return 0;
}

uint32_t If::Translate() {
	cond->Translate();
	int a1 = bco_ww1(bcout_g, IFNJ, 0);
	thenstm->Translate();
	if (elsestm) {
		int a2 = bco_ww1(bcout_g, JU, 0);
		bco_fix_forward_jmpw(bcout_g, a1);
		elsestm->Translate();
		bco_fix_forward_jmpw(bcout_g, a2);
	}
	else {
		bco_fix_forward_jmpw(bcout_g, a1);
	}

	return 0;
}

uint32_t While::Translate() {
	int a1 = bco_get_ip(bcout_g);
	cond->Translate();
	int a2 = bco_ww1(bcout_g, IFNJ, 0);

	int b1 = bco_get_ip(bcout_g);
	body->Translate();
	int b2 = bco_get_ip(bcout_g);
	
	bco_ww1(bcout_g, JU, a1);
	bco_resolve_break(bcout_g, b1, b2);
	bco_fix_forward_jmpw(bcout_g, a2);

	return 0;
}

uint32_t For::Translate() {
	init->Translate();
	int a1 = bco_get_ip(bcout_g);
	cond->Translate();
	int a2 = bco_ww1(bcout_g, IFNJ, 0);

	int b1 = bco_get_ip(bcout_g);
	body->Translate();
	int b2 = bco_get_ip(bcout_g);

	counter->Translate();

	bco_ww1(bcout_g, JU, a1);
	bco_resolve_break(bcout_g, b1, b2);
	bco_fix_forward_jmpw(bcout_g, a2);

	return 0;
}

uint32_t Try::Translate() {
	int try_addr = bco_get_ip(bcout_g);
	try_block->Translate();
	int a1 = bco_ww1(bcout_g, JU, 0);

	int catch_addr = bco_get_ip(bcout_g);
	catch_block->Translate();
	bco_fix_forward_jmpw(bcout_g, a1);

	bco_exinfo_add_block(bcout_g,
			mthEnv->exception_info_idx, try_addr, catch_addr);

	return 0;
}

uint32_t Throw::Translate() {
	obj->Translate();
	bco_w0(bcout_g, THROW);

	return 0;
}

uint32_t StatmList::Translate() {
	StatmList *s = this;
	do {
		s->statm->Translate();
		s = s->next;
	} while (s);

	return 0;
}

uint32_t Break::Translate() {
	bco_ww1_labeled(bcout_g, JU, 0, BRK);
	return 0;
}

uint32_t Prog::Translate() {
	lst->Translate();
	return 0;
}

uint32_t Class::Translate() {
	if (stm) {
		stm->Translate();
	}
	return 0;
}

uint32_t ClassList::Translate() {
	ClassList * lst = this;
	do {
		lst->cls->Translate();
		lst = lst->next;
	} while (lst);
	return 0;
}

uint32_t Method::Translate() {
	mthEnv->bc_entrypoint = bco_get_ip(bcout_g);

	StatmList *s = body;
	while (s->next) {
		s = s->next;
	}

	// Add implicit return if missing
	if (!dynamic_cast<Return*>(s->statm)) {
		if (isConstructor) {
			s->next = new StatmList(new Return(NULL), NULL);
		}
		else {
			s->next = new StatmList(new Return(new Nil), NULL);
		}
	}

	if (mthEnv->exobjs != NULL) {
		uint32_t exinfo_idx = bco_exinfo(bcout_g, mthEnv->try_block_cnt);
		bco_ww1(bcout_g, ST_EXINFO, exinfo_idx);
		mthEnv->exception_info_idx = exinfo_idx;
	}

	body->Translate();
	return 0;
}

uint32_t Return::Translate() {
	if (!expr) {
		bco_w0(bcout_g, RET_SELF);
	}
	else {
		expr->Translate();
		bco_w0(bcout_g, RET);
	}
	return 0;
}

CaseBlockScope::CaseBlockScope(CaseBlockScope *next_) {
	this->lo = NULL;
	this->eq = NULL;
	this->hi = NULL;
	this->type = 0;
	this->next = next_;
}

CaseBlockScope::CaseBlockScope(CaseBlockScope *next_, Expr *eq_) {
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

Node* CaseBlock::Optimize() {
	if (statmList) {
		statmList->Optimize();
	}
	if (next) {
		next->Optimize();
	}

	return this;
}

uint32_t CaseBlock::Translate() {
	if (this->statmList != NULL) {
		this->statmList->Translate();
	} else {
	}

	return 0;
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
				case 1: /* number */ {
					Numb *nEq = dynamic_cast<Numb*>(caseBlockScope_p->eq);
					if (nEq && numbTest->Value() == nEq->Value()) {
						caseBlock_p->statmList =
								(Statm *) (caseBlock_p->statmList->Optimize());

						return caseBlock_p;
					}

					break;
				}
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

uint32_t Case::Translate() {
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
				jumpBeforeBlock[jbbp++] = bco_ww1(bcout_g, JU, 0);
				break;
			}
			case 1: /* number */{
				/* I need a negation of == */
				Expr *rangeEq = new Bop(NotEq, this->expr, caseBlockScope->eq);
				rangeEq->Translate();

				/* if the bop made false from negation, so true from what we
				 * need, jump before block! */
				jumpBeforeBlock[jbbp++] = bco_ww1(bcout_g, IFNJ, 0);

				break;
			}
			case 2: /* range */{
				/* I __DO NOT__ need a negation of >= */
				Expr *rangeLo = new Bop(GreaterOrEq, this->expr,
						caseBlockScope->lo);
				rangeLo->Translate();

				/* if test of low rage fails, jump over the test of high range */
				int loRangeFail = bco_ww1(bcout_g, IFNJ, 0);

				/* I need a negation of <= */
				Expr *rangeHi = new Bop(Greater, this->expr,
						caseBlockScope->hi);
				rangeHi->Translate();

				jumpBeforeBlock[jbbp++] = bco_ww1(bcout_g, IFNJ, 0);

				/* */
				bco_fix_forward_jmpw(bcout_g, loRangeFail);

				break;
			}
			default:
				break;
			}

		}

		/* if we get here, we have not matched any scope! */
		int jumpOver = bco_ww1(bcout_g, JU, 0);

		/* if something matches, jump before the block */
		for (int i = 0; i < jbbp; i = i + 1) {
			bco_fix_forward_jmpw(bcout_g, jumpBeforeBlock[i]);
		}
		jbbp = 0;

		/* if all case block has matched, run the block! */
		nextBlock->Translate();

		/* if we have matched and block was run, jump to the end of the case */
		if (nextBlock->next != NULL) {
			finalJumps[fjp++] = bco_ww1(bcout_g, JU, 0);
		}

		/*  */
		bco_fix_forward_jmpw(bcout_g, jumpOver);
	}

	/* after every matched scope, hop/skip/jump to the end of the case */
	for (int i = 0; i < fjp; i = i + 1) {
		bco_fix_forward_jmpw(bcout_g, finalJumps[i]);
	}

	return 0;
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

void String::Print(int ident) {
	printfi(ident, "String [value=\"%s\"]\n", this->value);
}

void Symbol::Print(int ident) {
	printfi(ident, "Symbol [value=\"%s\"]\n", this->value);
}

void Array::Print(int ident) {
	printfi(ident, "Array\n");

	printfi(ident, "elems:\n");
	if (this->elems) {
		this->elems->Print(ident + 1);
	}
}

void Nil::Print(int ident) {
	printfi(ident, "Nil\n");
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

void Try::Print(int ident) {
	if (this->try_block) {
		printfi(ident, "Try\n");
		this->try_block->Print(ident + 1);
	}

	if (this->catch_block) {
		printfi(ident, "Catch\n");
		this->catch_block->Print(ident + 1);
	}
}

void Throw::Print(int ident) {
	printfi(ident, "Throw\n");

	if (this->obj) {
		this->obj->Print(ident + 1);
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
	printfi(ident, "Class [name=%s]\n", name);

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
	printfi(ident, "Method [name=%s, %c]\n", name, isStatic ? 'S' : 'I');

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

void CaseBlockScope::Print(int ident) {
	switch (type) {
	case 1:
		printfi(ident, "eq:\n");
		this->eq->Print(ident + 1);
		break;
	case 2:
		printfi(ident, "lo:\n");
		this->lo->Print(ident + 1);
		printfi(ident, "hi:\n");
		this->hi->Print(ident + 1);
		break;
	default:;
	}

	printfi(ident, "next:\n");
	if (this->next) {
		this->next->Print(ident + 1);
	}
}

void CaseBlock::Print(int ident) {
	printfi(ident, "CaseBlock\n");

	printfi(ident, "scope:\n");
	if (this->scope) {
		this->scope->Print(ident + 1);
	}

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

int ArgList::Count() {
	ArgList * a = this;
	int count = 0;

	do {
		count++;
		a = a->next;
	} while (a);
	return count;
}