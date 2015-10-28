/* tabsym.cpp */

#include "tabsym.h"
#include "lexan.h"
#include "parser.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

CRecord::CRecord(char *ident) {
	this->m_Ident = strdup(ident);
	this->m_Val = volna_adr++;
	this->m_Next = NULL;
}

CRecord::CRecord(char *ident, CRecord *next) {
	this->m_Ident = strdup(ident);
	this->m_Val = volna_adr++;
	this->m_Next = next;
}

PrvekTab::PrvekTab(char *i, DruhId d, Scope s, int h, PrvekTab *n) {
	ident = new char[strlen(i)+1];
	strcpy(ident, i);
	druh = d; hodn = h; dalsi = n;
	sc = s;

	prvni = 0;
	pole = false;
}

PrvekTab::PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n){
   ident = new char[strlen(i)+1];
   strcpy(ident, i);
   druh = d; hodn = h; dalsi = n;

   prvni = f;
   posledni = l;
   pole = true;
}

ClassEnv::ClassEnv(char * name, ClassEnv * par, ClassEnv * n) {
	className = new char[strlen(name) + 1];
	strcpy(className, name);
	if (par) {
		parent = par;
	}
	else{
		parent = NULL;
	}
	next = n;
	class_addr_next = 0;
	obj_addr_next = 0;
	syms = NULL;
	methods = NULL;
	constructor = NULL;
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
}

PrvekTab *hledejId(char *id) {
	PrvekTab *p = TabSym;
	while (p)
		if (strcmp(id, p->ident) == 0)
			return p;
		else
			p = p->dalsi;
	return NULL;
}

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

MethodEnv * hledejMethod(char * id, ClassEnv * ce) {
	MethodEnv * me = ce->methods;

	while (me) {
		if(!strcmp(id, me->methodName)) {
			return me;
		}
		me = me->next;
	}
	return NULL;
}

PrvekTab * hledejMember(char * id, ClassEnv * ce, MethodEnv * me) {
	PrvekTab * syms;
	if (me) { // Try to search through local vars first
		syms = me->syms;

		while (syms) {
			if(!strcmp(id, syms->ident)) {
				return syms;
			}
			syms = syms->dalsi;
		}
	}

	// If we get here, we try class members
	syms = ce->syms;

	while (syms) {
		if(!strcmp(id, syms->ident)) {
			return syms;
		}
		syms = syms->dalsi;
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
		if (cls->constructor) {
			Chyba(mth, "trida muze mit pouze jeden konstruktor");
		}
		cls->constructor = new MethodEnv(mth, isStatic, NULL);
		return cls->constructor;
	}

	MethodEnv * me = hledejMethod(mth, cls);

	if (me) {
		Chyba(mth, "druha deklarace");
	}
	cls->methods = new MethodEnv(mth, isStatic, cls->methods);

	return cls->methods;
}

void deklKonst(char *id, int val, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth);
	if (p) {
		Chyba(id, "druha deklarace");
	}

	// Constant value doesn't need memory space - it will be inlined
	// Shouldn't we change it?
	if (mth) {
		mth->syms = new PrvekTab(id, IdKonst, SC_LOCAL, val, mth->syms);
	}
	else if (isStatic) {
		cls->syms = new PrvekTab(id, IdKonst, SC_CLASS, val, cls->syms);
	}
	else {
		Chyba(id, "Konstantni clen tridy musi byt staticky.");
	}
}

void deklProm(char *id, bool arg, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth);
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
		}
		else {
			addr = mth->local_addr_next;
			mth->local_addr_next++;
			sc = SC_LOCAL;
		}

		mth->syms = new PrvekTab(id, IdProm, sc, addr, mth->syms);
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
void deklProm(char *id, int prvni, int posledni, ClassEnv * cls, MethodEnv * mth){
	PrvekTab *p = hledejMember(id, cls, mth);
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
}

/*void deklRecord(char *id, CRecord *record) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
		return;
	}

	TabSym = new PrvekTab(id, IdRecord, 0, TabSym, record);
}*/

/*int adrProm(char *id) {
	PrvekTab *p = hledejId(id);
	if (!p) {
		Chyba(id, "neni deklarovan");
		return 0;
	} else if (p->druh == IdRecord) {
		Srovnani(DOT);

		char idMember[MAX_IDENT_LEN];
		Srovnani_IDENT(idMember);

		for (CRecord *r = p->record; r != NULL; r = r->m_Next) {
			if (strcmp(r->m_Ident, idMember) == 0) {
				return r->m_Val;
			}
		}

		printf("member of %s not found\n", p->ident);
		exit(1);

	} else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
		return 0;
	} else
		return p->hodn;
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

// TODO: fix this to use env and search in the right place
int prvniIdxProm(char *id, ClassEnv * cls, MethodEnv * mth) // Will probably also remove this - first index of array will always be zero
{
	PrvekTab *p = hledejMember(id, cls, mth);
	if (!p) {
		Chyba(id, "neni deklarovan");
	}
	else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
	}
	return p->prvni;
}

// TODO: fix this to use env and search in the right place
/*DruhId idPromKonst(char *id, int *v, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejMember(id, cls, mth);
	if (!p) {
		Chyba(id, "neni deklarovan");
	}
	return p->druh;
}*/

void symCleanup() {
	PrvekTab * sym = TabSym;
	PrvekTab * next;

	while(sym){
		next = sym->dalsi;
		delete sym;
		sym = next;
	}
}
