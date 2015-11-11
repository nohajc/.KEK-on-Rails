/* vstup.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 257

/**
 * staticky definované pole znaků kde se ukládají jednotlivé řádky
 */
char line[MAX_LINE_LENGTH];
int lineNumber = 0;
char *linePointer = line;
FILE *inputFileStack[1024];
int inputFileNum;
int extendedLine = 0;

/**
 * inicializace vstupu na standardní vstup nebo na vstup ze souboru
 * standardní vstup se zvolí pokud fileName je NULL
 */
int initInput(char* fileName) {
	if (!fileName) {
		inputFileStack[0] = stdin;
	} else {
		inputFileNum = 0;
		inputFileStack[0] = fopen(fileName, "r");
		if (!inputFileStack[0]) {
			printf("Vstupni soubor %s nenalezen.\n", fileName);
			return 0;
		}
	}
	return 1;
}

void closeInput() {
	while (inputFileNum--) {
		fclose(inputFileStack[inputFileNum]);
	}
}

char * getLine(char * buf, int maxLen) {
	FILE * f;
	char * str = fgets(buf, MAX_LINE_LENGTH, inputFileStack[inputFileNum]);
	while (!str) {
		if (inputFileNum > 0) {
			inputFileNum--;
			str = fgets(buf, MAX_LINE_LENGTH, inputFileStack[inputFileNum]);
		}
		else {
			return NULL;
		}
	}
	if (!strncmp(str, "#include", 8)) {
		char incl[16], expr[257], * fname;
		sscanf(str, "%s %s", incl, expr);
		if (expr[0] != '\"' || expr[strlen(expr) - 1] != '\"') {
			printf("Chybna syntaxe ve vyrazu #include.\n");
			exit(1);
		}
		expr[strlen(expr) - 1] = '\0';
		fname = expr + 1;

		f = fopen(fname, "r");
		if (!f) {
			printf("Soubor %s nenalezen.\n", fname);
			exit(1);
		}
		inputFileStack[++inputFileNum] = f;
		str = getLine(buf, maxLen);
	}
	return str;
}

/**
 * přečte jeden symbol ze vstupu
 */
int getChar() {
	if (!*linePointer) {
		if (!getLine(line, MAX_LINE_LENGTH))
			return EOF;

		linePointer = line;
		lineNumber++;

		int lineLength = strlen(line);
		if (extendedLine) {
			lineNumber--;
			printf("+    %s", line);
		} else {
			printf("%-4d %s", lineNumber, line);
		}

		extendedLine = line[lineLength - 1] != '\n';
	}
	return *linePointer++;
}
