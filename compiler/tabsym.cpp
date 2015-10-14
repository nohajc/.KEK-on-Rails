/* tabsym.cpp */

#include "tabsym.h"
#include <string.h>
#include <stdio.h>

struct PrvekTab {
	char *ident;
	DruhId druh;
	int hodn;
	PrvekTab *dalsi;
	PrvekTab(char *i, DruhId d, int h, PrvekTab *n);
};

PrvekTab::PrvekTab(char *i, DruhId d, int h, PrvekTab *n) {
	ident = new char[strlen(i) + 1];
	strcpy(ident, i);
	druh = d;
	hodn = h;
	dalsi = n;
}

static PrvekTab *TabSym;

static void Chyba(char *id, char *txt) {
	printf("identifikator %s: %s\n", id, txt);
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

void deklKonst(char *id, int val) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
		return;
	}
	TabSym = new PrvekTab(id, IdKonst, val, TabSym);
}

void deklProm(char *id) {
	static int volna_adr;
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
		return;
	}
	TabSym = new PrvekTab(id, IdProm, volna_adr, TabSym);
	volna_adr++;
}

int adrProm(char *id) {
	PrvekTab *p = hledejId(id);
	if (!p) {
		Chyba(id, "neni deklarovan");
		return 0;
	} else if (p->druh != IdProm) {
		Chyba(id, "neni identifikatorem promenne");
		return 0;
	} else
		return p->hodn;
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

