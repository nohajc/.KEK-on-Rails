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

PrvekTab::PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r) {
	ident = strdup(i);

	druh = d;
	dalsi = n;

	switch (d) {
	case IdKonst:
		hodn = h;
		break;
	case IdProm:
		this->hodn = volna_adr++;
		break;
	case IdRecord:
		this->record = r;
		break;
	default:
		break;
	}
}

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
	TabSym = new PrvekTab(id, IdKonst, val, TabSym, NULL);
}

void deklProm(char *id) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
		return;
	}
	TabSym = new PrvekTab(id, IdProm, volna_adr, TabSym, NULL);
	volna_adr++;
}

void deklRecord(char *id, CRecord *record) {
	PrvekTab *p = hledejId(id);
	if (p) {
		Chyba(id, "druha deklarace");
		return;
	}

	TabSym = new PrvekTab(id, IdRecord, 0, TabSym, record);
}

int adrProm(char *id) {
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

