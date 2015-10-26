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

PrvekTab::PrvekTab(char *i, DruhId d, int h, PrvekTab *n) {
	ident = new char[strlen(i)+1];
	strcpy(ident, i);
	druh = d; hodn = h; dalsi = n;

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
	next = n;
}

ClassEnv::~ClassEnv() {
	delete [] className;
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

MethodEnv * hledejMethod(char *, ClassEnv *) {

}

PrvekTab * hledejMember(char *, ClassEnv *, MethodEnv *) {

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
	return NULL; // TODO: Implement
}

void deklKonst(char *id, int val, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
	}
	TabSym = new PrvekTab(id, IdKonst, val, TabSym);
}

void deklProm(char *id, bool arg, bool isStatic, ClassEnv * cls, MethodEnv * mth) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
	}
	TabSym = new PrvekTab(id, IdProm, volna_adr, TabSym);
	volna_adr++;
}

// Static array
void deklProm(char *id, int prvni, int posledni, ClassEnv * cls, MethodEnv * mth){
   PrvekTab *p = hledejId(id);
   if (p) {
      Chyba(id, "druha deklarace");
   }

   //printf("Storing %s at address %d.\n", id, volna_adr);
   TabSym = new PrvekTab(id, IdProm, volna_adr, prvni, posledni, TabSym);
   volna_adr += posledni - prvni + 1;
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

int adrProm(char *id) {
	PrvekTab *p = hledejId(id);
	if (!p) {
		Chyba(id, "neni deklarovan");
		exit(1);
	}
	else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
		exit(1);
	}
	else{
		return p->hodn;
	}
}

int prvniIdxProm(char *id)
{
	PrvekTab *p = hledejId(id);
	if (!p) {
		Chyba(id, "neni deklarovan");
		exit(1);
	}
	else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
		exit(1);
	}
	else{
		return p->prvni;
	}
}

DruhId idPromKonst(char *id, int *v) {
	PrvekTab *p = TabSym;
	while (p)
		if (strcmp(id, p->ident) == 0) {
			*v = p->hodn;
			return p->druh;
		} else
			p = p->dalsi;
	Chyba(id, "neni deklarovan");
	return Nedef;
}

void symCleanup() {
	PrvekTab * sym = TabSym;
	PrvekTab * next;

	while(sym){
		next = sym->dalsi;
		delete sym;
		sym = next;
	}
}
