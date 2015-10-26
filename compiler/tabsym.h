/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

#include <stdio.h>
#include <stdlib.h>

enum DruhId {
	Nedef, IdProm, IdKonst, IdRecord
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

struct PrvekTab {
	char *ident;
	DruhId druh;
	int hodn;
	bool pole;
	bool isStatic;
	int prvni, posledni;

	//CRecord *record;
	PrvekTab *dalsi;
	//PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);

	// Variable
	PrvekTab(char *i, DruhId d, int h, PrvekTab *n);
	// Static array
	PrvekTab(char *i, DruhId d, int h, int f, int l, PrvekTab *n);
	~PrvekTab();
};

struct MethodEnv {
	char * methodName;
	bool isStatic;
	PrvekTab * syms; // method vars and consts
	MethodEnv * next;
};

struct ClassEnv {
	char * className;
	PrvekTab * syms; // class consts and vars
	MethodEnv * methods;
	ClassEnv * next;
};

struct Env {
	ClassEnv * clsEnv;
	MethodEnv * mthEnv;
};

static ClassEnv * TabClass;


static PrvekTab * TabSym; // TODO: remove this
static int volna_adr;

ClassEnv * deklClass(char *);
void deklKonst(char *, int, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
void deklProm(char *, int, int, ClassEnv * cls = NULL, MethodEnv * mth = NULL);
//void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *);

int adrProm(char*);
int prvniIdxProm(char *id);
DruhId idPromKonst(char*, int*);

void symCleanup();

#endif
