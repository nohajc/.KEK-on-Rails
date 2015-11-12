/* main.c */
/* syntakticky analyzator */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lexan.h"
#include "parser.h"
#include "strom.h"
#include "bcout.h"


int main(int argc, char *argv[]) {
	char *fileName;
	char outName[256];
	int ast_print;
	Prog *prog;

	ast_print = 1;
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

	bcout_g = bcout_init();

	prog = Program();
	if(!prog){
		printf("Vstupni soubor je prazdny.\n");
		return 0;
	}

	prog = (Prog*) (prog->Optimize());

	if (ast_print) {
		prog->Print(0);
	}

	prog->Translate();

	assert(TabClass != NULL);

	if (fileName) {
		char * name = strtok(fileName, ".");
		sprintf(outName, "%s.kexe", name);
		bcout_to_file(bcout_g, TabClass, outName);
	}
	else {
		bcout_to_file(bcout_g, TabClass, "out.kexe");
	}

	closeInput();
	delete prog;

	return 0;
}
