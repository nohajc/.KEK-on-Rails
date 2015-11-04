/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

#include <stdio.h>
#include <stdlib.h>

enum DruhId {
	Nedef, IdProm, IdConstNum, IdConstStr
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
	int hodn; // TODO: hodn will always be address (even for consts)
	//int prvni, posledni;
	char * str_val; // TODO: here will be a pointer to constant pool (in case of ConstNum, ConstStr)

	//CRecord *record;
	PrvekTab *dalsi;
	//PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);

	// Variable / constant int
	PrvekTab(char *i, DruhId d, Scope s, int h, PrvekTab *n);
	// const str
	PrvekTab(char *i, DruhId d, Scope s, const char * val, PrvekTab *n);
	//PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n);
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
	unsigned int bc_entrypoint; // Bytecode address

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

// Special class environment for searching through all classes
#if defined(__LP64__)
#define CLASS_ANY (ClassEnv*)0xFFFFFFFFFFFFFFFFULL
#else
#define CLASS_ANY (ClassEnv*)0xFFFFFFFF
#endif

struct Env {
	ClassEnv * clsEnv;
	MethodEnv * mthEnv;
	bool self;
};

static ClassEnv * TabClass = NULL;

/*static PrvekTab * TabSym; // TODO: remove this
static int volna_adr;*/

ClassEnv * deklClass(char *, char * = NULL);
MethodEnv * deklMethod(char *, bool constructor = false, bool isStatic = false, ClassEnv * cls = NULL);
// const int
void deklKonst(char *, int, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
// const string
void deklKonst(char *, char *, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, bool arg = false, bool isStatic = false, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
//void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *); // TODO: remove this
ClassEnv * hledejClass(char *);
MethodEnv * hledejMethod(char *, ClassEnv *, bool = true);
PrvekTab * hledejMember(char *, ClassEnv *, MethodEnv *, bool = true);

PrvekTab * adrSym(char*, ClassEnv *, MethodEnv *);
PrvekTab * adrProm(char*, ClassEnv *, MethodEnv *);
int prvniIdxProm(char *, ClassEnv *, MethodEnv *);
//DruhId idPromKonst(char *, int *, ClassEnv *, MethodEnv *);

void symCleanup();

#endif
