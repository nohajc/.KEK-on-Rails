/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "vm.h"

void usage(const char *progname) {
	printf("%s -f [filename]\n", progname);
}

int main(int argc, char *argv[]) {
	int c;
	FILE *f;
	char *file_name = NULL;

	class_t *cl;
	method_t *me;

	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch (c) {
		case 'f':
			file_name = optarg;
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

	if (!file_name) {
		usage(argv[0]);
	}

	cl = class_find("Main");
	me = method_find(cl, "main");
	method_eval(me);

	return (EXIT_SUCCESS);
}
