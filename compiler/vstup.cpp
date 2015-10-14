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
FILE *inputFile;
int extendedLine = 0;

/**
 * inicializace vstupu na standardní vstup nebo na vstup ze souboru
 * standardní vstup se zvolí pokud fileName je NULL
 */
int initInput(char* fileName) {
	if (!fileName) {
		inputFile = stdin;
	} else {
		inputFile = fopen(fileName, "rt");
		if (!inputFile) {
			printf("Vstupni soubor %s nenalezen.\n", fileName);
			return 0;
		}
	}
	return 1;
}

/**
 * přečte jeden symbol ze vstupu
 */
int getChar() {
	if (!*linePointer) {
		if (!fgets(line, MAX_LINE_LENGTH, inputFile))
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
