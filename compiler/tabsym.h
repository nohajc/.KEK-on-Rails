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
	bool pole;
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

static PrvekTab *TabSym;
static int volna_adr;

void deklKonst(char *, int);
void deklProm(char *);
void deklProm(char *, int, int);
void deklRecord(char *, CRecord *);

PrvekTab *hledejId(char *);

int adrProm(char*);
int prvniIdxProm(char *id);
DruhId idPromKonst(char*, int*);

void symCleanup();

#endif
