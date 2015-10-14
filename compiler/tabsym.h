/* tabsym.h */

enum DruhId {
	Nedef, IdProm, IdKonst
};

void deklKonst(char*, int);
void deklProm(char*);
int adrProm(char*);
DruhId idPromKonst(char*, int*);

