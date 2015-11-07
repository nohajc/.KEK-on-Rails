/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "loader.h"
#include "vm.h"

void usage(const char *progname) {
	printf("Usage:\n");
	printf("%s -f [filename]\n", progname);
	exit(1);
}

int main(int argc, char *argv[]) {
	int c;
	char *filename = NULL;

	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch (c) {
		case 'f':
			filename = optarg;
			break;
		case '?':
			fprintf(stderr, "unknown opt: \"%c\"\n", c);
			return (EXIT_FAILURE);
			break;
		default:
			abort();
			break;
		}
	}

	if (filename == NULL) {
		usage(argv[0]);
	}

	kexe_load(filename);

	return (EXIT_SUCCESS);
}