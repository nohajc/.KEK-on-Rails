/* main.c */
/* syntakticky analyzator */

#include "lexan.h"
#include "parser.h"
#include "strom.h"
#include "zaspoc.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
	char *fileName;
	printf("Syntakticky analyzator\n");
	if (argc == 1) {
		printf("Vstup z klavesnice, zadejte zdrojovy text\n");
		fileName = NULL;
	} else {
		fileName = argv[1];
		printf("Vstupni soubor %s\n", fileName);
	}
	if (!initParser(fileName)) {
		printf("Chyba pri vytvareni syntaktickeho analyzatoru.\n");
		return 0;
	}
	Prog *prog = Program();
	prog = (Prog*) (prog->Optimize());
	prog->Translate();
	Print();
	Run();
	printf("Konec\n");
	closeInput();
	delete prog;
	return 0;
}
