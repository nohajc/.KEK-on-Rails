/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

#include <stdio.h>
#include <stdlib.h>

enum DruhId {
	Nedef, IdProm, IdKonst
};

enum Scope {
	SC_GLOBAL, // Only class names are global - you can globally reference a static class member
	SC_LOCAL,
	SC_ARG,
	SC_INSTANCE, // instance variable
	SC_CLASS // class static variable
};

class CRecord {
public:
	char *m_Ident;
	int m_Val;
	CRecord *m_Next;

	CRecord(char *ident);
	CRecord(char *ident, CRecord *next);
	/* TODO: destructor */
};

struct ClassEnv;

struct PrvekTab {
	char *ident;
	DruhId druh;
	Scope sc;
	int hodn; // TODO: this will be a pointer (var can also be an obj ref)
	bool pole;
	int prvni, posledni;

	//CRecord *record;
	PrvekTab *dalsi;
	//PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);

	// Variable
	PrvekTab(char *i, DruhId d, Scope s, int h, PrvekTab *n);
	// Static array
	PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n);
	~PrvekTab();
};

struct MethodEnv {
	char * methodName;
	bool isStatic;
	PrvekTab * args; // method arguments
	PrvekTab * syms; // method vars and consts
	MethodEnv * next;
	int arg_addr_next; // Arguments address counter
	int local_addr_next; // Local vars address counter

	MethodEnv(char * name, bool sttc, MethodEnv * n = NULL);
	~MethodEnv();
};

struct ClassEnv {
	char * className;
	PrvekTab * syms; // class consts and vars
	MethodEnv * methods;
	MethodEnv * constructor;
	ClassEnv * next;
	ClassEnv * parent;
	int class_addr_next; // Static members address counter
	int obj_addr_next; // Instance members address counter

	ClassEnv(char * name, ClassEnv * par, ClassEnv * n);
	~ClassEnv();
};

struct Env {
	ClassEnv * clsEnv;
	MethodEnv * mthEnv;
};

static ClassEnv * TabClass = NULL;

static PrvekTab * TabSym; // TODO: remove this
static int volna_adr;

ClassEnv * deklClass(char *, char * = NULL);
MethodEnv * deklMethod(char *, bool constructor = false, bool isStatic = false, ClassEnv * cls = NULL);
void deklKonst(char *, int, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, bool arg = false, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, int, int, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
//void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *); // TODO: remove this
ClassEnv * hledejClass(char *);
MethodEnv * hledejMethod(char *, ClassEnv *);
PrvekTab * hledejMember(char *, ClassEnv *, MethodEnv *);

PrvekTab * adrSym(char*, ClassEnv *, MethodEnv *);
PrvekTab * adrProm(char*, ClassEnv *, MethodEnv *);
int prvniIdxProm(char *, ClassEnv *, MethodEnv *);
//DruhId idPromKonst(char *, int *, ClassEnv *, MethodEnv *);

void symCleanup();

#endif
