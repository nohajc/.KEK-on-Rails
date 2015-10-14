/* tabsym.h */

#ifndef TABSYM_H
#define TABSYM_H

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
	CRecord *record;
	PrvekTab *dalsi;
	PrvekTab(char *i, DruhId d, int h, PrvekTab *n, CRecord *r);
};

static PrvekTab *TabSym;
static int volna_adr;

void deklKonst(char *, int);
void deklProm(char *);
void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *);

int adrProm(char*);
DruhId idPromKonst(char*, int*);

#endif
