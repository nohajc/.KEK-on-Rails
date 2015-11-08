/* tabsym.cpp */

#include "tabsym.h"
#include "lexan.h"
#include "parser.h"
#include "bcout.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* global variable */
ClassEnv *TabClass;

/*CRecord::CRecord(char *ident) {
	this->m_Ident = strdup(ident);
	this->m_Val = volna_adr++;
	this->m_Next = NULL;
}

CRecord::CRecord(char *ident, CRecord *next) {
	this->m_Ident = strdup(ident);
	this->m_Val = volna_adr++;
	this->m_Next = next;
}*/

PrvekTab::PrvekTab(char *i, DruhId d, Scope s, int a, PrvekTab *n) {
	ident = new char[strlen(i)+1];
	strcpy(ident, i);
	druh = d; addr = a; dalsi = n;
	sc = s;
	const_ptr = 0;
	val.str = NULL;
}

PrvekTab::PrvekTab(char *i, DruhId d, Scope s, int a, int v, PrvekTab *n) {
	ident = new char[strlen(i)+1];
	strcpy(ident, i);
	druh = d; addr = a; dalsi = n;
	sc = s;
	val.num = v;
	const_ptr = bco_int(bcout_g, v);
}

PrvekTab::PrvekTab(char *i, DruhId d, Scope s, int a, const char * v, PrvekTab *n) {
	ident = new char[strlen(i)+1];
	strcpy(ident, i);
	druh = d; addr = a; dalsi = n;
	sc = s;
	val.str = new char[strlen(v)+1];
	strcpy(val.str, v);
	const_ptr = bco_str(bcout_g, v);
}

/*PrvekTab::PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n){
   ident = new char[strlen(i)+1];
   strcpy(ident, i);
   druh = d; hodn = h; dalsi = n;

   prvni = f;
   posledni = l;
   pole = true;
}*/

ClassEnv::ClassEnv(char * name, ClassEnv * par, ClassEnv * n) {
	className = new char[strlen(name) + 1];
	strcpy(className, name);
	parent = par;
	next = n;
	if (parent) {
		class_addr_next = parent->class_addr_next;
		obj_addr_next = parent->obj_addr_next;
	}
	else {
		class_addr_next = 0;
		obj_addr_next = 0;	
	}
	syms = NULL;
	methods = NULL;
	constructor = NULL;
	static_init = NULL;
}

ClassEnv::~ClassEnv() {
	delete [] className;
}

MethodEnv::MethodEnv(char * name, bool sttc, MethodEnv * n) {
	methodName = new char[strlen(name) + 1];
	strcpy(methodName, name);
	isStatic = sttc;
	next = n;
	arg_addr_next = 0;
	local_addr_next = 0;
	args = NULL;
	syms = NULL;
	bc_entrypoint = 0; /* TODO FIXME */
}

MethodEnv::~MethodEnv() {
	delete [] methodName;
}

static void Chyba(char *id, char *txt) {
	printf("identifikator %s: %s\n", id, txt);
	exit(1);
}

PrvekTab::~PrvekTab() {
	delete [] ident;
	if (druh == IdConstStr) {
		delete [] val.str;
	}
}

/*PrvekTab *hledejId(char *id) {
	PrvekTab *p = TabSym;
	while (p)
		if (strcmp(id, p->ident) == 0)
			return p;
		else
			p = p->dalsi;
	return NULL;
}*/

ClassEnv * hledejClass(char * id) {
	ClassEnv * ce = TabClass;
	while (ce) {
		if (!strcmp(id, ce->className)) {
			return ce;
		}
		ce = ce->next;
	}
	return NULL;
}

#define FIND_METHOD(first) { \
	me = first; \
	while (me) { \
		if(!strcmp(id, me->methodName)) { \
			return me; \
		} \
		me = me->next; \
	} \
}

MethodEnv * hledejMethod(char * id, ClassEnv * ce, bool recursive) {
	MethodEnv * me;

	if(ce == CLASS_ANY) {
		ClassEnv * tc = TabClass;
		while (tc) {
			FIND_METHOD(tc->methods);
			tc = tc->next;
		}
	}

	FIND_METHOD(ce->methods);

	if (recursive && ce->parent) {
		return hledejMethod(id, ce->parent);
	}
	return NULL;
}

#define FIND_SYM(first) { \
	syms = first; \
	while (syms) { \
		if(!strcmp(id, syms->ident)) { \
			return syms; \
		} \
		syms = syms->dalsi; \
	} \
}

PrvekTab * hledejMember(char * id, ClassEnv * ce, MethodEnv * me, bool recursive) {
	PrvekTab * syms;
	if (me) {
		FIND_SYM(me->syms);	// Try to search through local vars first
		FIND_SYM(me->args); // Then try method args
	}

	if (!recursive) return NULL;

	// If we get here, we try class members
	if (ce == CLASS_ANY) {
		ClassEnv * tc = TabClass;
		while (tc) {
			FIND_SYM(tc->syms);
			tc = tc->next;
		}
	}

	FIND_SYM(ce->syms);

	if (ce->parent) {
		return hledejMember(id, ce->parent, NULL);
	}
	return NULL;
}

ClassEnv * deklClass(char * cls, char * par) {
	ClassEnv * ce = hledejClass(cls);
	ClassEnv * pe = NULL;

	if (par) {
		pe = hledejClass(par);
		if (!pe) {
			Chyba(par, "neni deklarovan");
		}
	}
	
	if (ce) {
		Chyba(cls, "druha deklarace");
	}
	TabClass = new ClassEnv(cls, pe, TabClass);
	return TabClass;
}

MethodEnv * deklMethod(char * mth, bool constructor, bool isStatic, ClassEnv * cls){
	if (constructor) {
		if (!isStatic) {
			if (cls->constructor) {
				Chyba(mth, "trida muze mit pouze jeden konstruktor");
			}
			cls->constructor = new MethodEnv(mth, isStatic, NULL);
			return cls->constructor;
		}

		if (cls->static_init) {
			Chyba(mth, "trida muze mit pouze jeden staticky inicializator");
		}
		cls->static_init = new MethodEnv(mth, isStatic, NULL);
	}

	MethodEnv * me = hledejMethod(mth, cls, false);

	if (me) {
		Chyba(mth, "druha deklarace");
	}
	cls->methods = new MethodEnv(mth, isStatic, cls->methods);

	return cls->methods;
}

void deklKonst(char *id, int val, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth, false);
	if (p) {
		Chyba(id, "druha deklarace");
	}

	if (mth) {
		int addr;
		addr = mth->local_addr_next;
		mth->local_addr_next++;

		mth->syms = new PrvekTab(id, IdConstNum, SC_LOCAL, addr, val, mth->syms);
	}
	else if (isStatic) {
		int addr;
		addr = cls->class_addr_next;
		cls->class_addr_next++;

		cls->syms = new PrvekTab(id, IdConstNum, SC_CLASS, addr, val, cls->syms);
	}
	else {
		Chyba(id, "Konstantni clen tridy musi byt staticky.");
	}
}

void deklKonst(char *id, char * val, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth, false);
	if (p) {
		Chyba(id, "druha deklarace");
	}

	if (mth) {
		int addr;
		addr = mth->local_addr_next;
		mth->local_addr_next++;

		mth->syms = new PrvekTab(id, IdConstStr, SC_LOCAL, addr, val, mth->syms);
	}
	else if (isStatic) {
		int addr;
		addr = cls->class_addr_next;
		cls->class_addr_next++;

		cls->syms = new PrvekTab(id, IdConstStr, SC_CLASS, addr, val, cls->syms);
	}
	else {
		Chyba(id, "Konstantni clen tridy musi byt staticky.");
	}
}

void deklProm(char *id, bool arg, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth, false);
	if (p) {
		Chyba(id, "druha deklarace");
	}

	if (mth) { // Local or args
		int addr;
		Scope sc;

		if (arg) {
			addr = mth->arg_addr_next;
			mth->arg_addr_next++;
			sc = SC_ARG;

			mth->args = new PrvekTab(id, IdProm, sc, addr, mth->args);
		}
		else {
			addr = mth->local_addr_next;
			mth->local_addr_next++;
			sc = SC_LOCAL;

			mth->syms = new PrvekTab(id, IdProm, sc, addr, mth->syms);
		}
	}
	else { // Class or instance
		int addr;
		Scope sc;

		if (isStatic) {
			addr = cls->class_addr_next;
			cls->class_addr_next++;
			sc = SC_CLASS;
		}
		else {
			addr = cls->obj_addr_next;
			cls->obj_addr_next++;
			sc = SC_INSTANCE;
		}

		cls->syms = new PrvekTab(id, IdProm, sc, addr, cls->syms);
	}
}

// Static array - TODO: remove, it is broken
/*void deklProm(char *id, int prvni, int posledni, ClassEnv * cls, MethodEnv * mth){
	PrvekTab *p = hledejMember(id, cls, mth, false);
	if (p) {
		Chyba(id, "druha deklarace");
	}

	//printf("Storing %s at address %d.\n", id, volna_adr);
	if (mth) {
		mth->syms = new PrvekTab(id, IdProm, mth->local_addr_next, prvni, posledni, mth->syms);
		mth->local_addr_next += posledni - prvni + 1;
	}
	else{
		cls->syms = new PrvekTab(id, IdProm, cls->obj_addr_next, prvni, posledni, cls->syms);
		cls->obj_addr_next += posledni - prvni + 1;
	}
}*/

PrvekTab * adrSym(char *id, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth); // local, instance or class var
	if (!p) {
		Chyba(id, "neni deklarovan");
		exit(1);
	}
	else{
		return p;
	}
}

PrvekTab * adrProm(char *id, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab * p = adrSym(id, cls, mth);
	if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
	}
	return p;
}

/*int prvniIdxProm(char *id, ClassEnv * cls, MethodEnv * mth) // Will probably also remove this - first index of array will always be zero
{
	PrvekTab *p = hledejMember(id, cls, mth);
	if (!p) {
		Chyba(id, "neni deklarovan");
	}
	else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
	}
	return p->prvni;
}*/

/*DruhId idPromKonst(char *id, int *v, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth);
	if (!p) {
		Chyba(id, "neni deklarovan");
	}
	return p->druh;
}*/

void symCleanup() { // TODO: fix this
	PrvekTab * sym = NULL;
	PrvekTab * next;

	while(sym){
		next = sym->dalsi;
		delete sym;
		sym = next;
	}
}
