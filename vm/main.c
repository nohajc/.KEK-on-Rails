/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "loader.h"
#include "memory.h"
#include "vm.h"

uint32_t debug_level_g = 0;
uint32_t test_g = 0;
gc_type_t gc_type_g = GC_TYPE_DEFAULT;

void usage(const char *progname) {
	printf("Usage:\n");
	printf("%s -d [s|sf|l|b] -f [filename]\n", progname);
	exit(EXIT_FAILURE);
}

void free_globals() {
	uint32_t i;
	uint32_t j;

	if (classes_g != NULL) {
		for (i = 0; i < classes_cnt_g; i++) {
			class_free(&classes_g[i]);
		}
	}

	gc_delete_all();

	free(bc_arr_g);
	free(const_table_g);
	free(classes_g);
	stack_destroy();
}

static void debug_add(char *type) {
	if (strcmp(type, "s") == 0 || strcmp(type, "stack") == 0) {
		debug_level_g |= DBG_STACK;
	} else if (strcmp(type, "sf") == 0 || strcmp(type, "stack_full") == 0) {
		debug_level_g |= DBG_STACK_FULL;
	} else if (strcmp(type, "l") == 0 || strcmp(type, "loading") == 0) {
		debug_level_g |= DBG_LOADING;
	} else if (strcmp(type, "b") == 0 || strcmp(type, "bc") == 0) {
		debug_level_g |= DBG_BC;
	} else if (strcmp(type, "v") == 0 || strcmp(type, "vm") == 0) {
		debug_level_g |= DBG_VM;
	} else if (strcmp(type, "a") == 0 || strcmp(type, "all") == 0) {
		debug_level_g |= DBG_ALL;
	} else if (strcmp(type, "g") == 0 || strcmp(type, "gc") == 0) {
		debug_level_g |= DBG_GC;
	} else if (strcmp(type, "m") == 0 || strcmp(type, "mem") == 0) {
		debug_level_g |= DBG_MEM;
	} else if (strcmp(type, "o") == 0 || strcmp(type, "obj_tbl") == 0) {
		debug_level_g |= DBG_OBJ_TBL;
	} else if (strcmp(type, "s") == 0 || strcmp(type, "gc_stats") == 0) {
		debug_level_g |= DBG_GC_STATS;
	} else if (strcmp(type, "f") == 0 || strcmp(type, "fc") == 0) {
		debug_level_g |= DBG_FC;
	} else {
		fprintf(stderr, "Unknown debug level \"%s\"\n", type);
		exit(1);
	}
}

static void set_gc(char *type) {
	if (strcmp(type, "x") == 0 || strcmp(type, "none") == 0) {
		gc_type_g = GC_NONE;
	} else if (strcmp(type, "n") == 0 || strcmp(type, "new") == 0) {
		gc_type_g = GC_NEW;
	} else if (strcmp(type, "g") == 0 || strcmp(type, "gen") == 0) {
		gc_type_g = GC_GEN;
	} else {
		fprintf(stderr, "Unknown gc type \"%s\" (use none, new or gen)\n",
				type);
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	int c;
	char *filename = NULL;

	while ((c = getopt(argc, argv, "d:g:t:")) != -1) {
		switch (c) {
		case 'd':
			debug_add(optarg);
			break;
		case 'g':
			set_gc(optarg);
			break;
		case 't':
			sscanf(optarg, "%u", &test_g);
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

	if (optind == argc) {
		usage(argv[0]);
	}

	filename = argv[optind];

	if (!mem_init()) {
		fprintf(stderr, "Memory initialization has failed.\n");
		return (EXIT_FAILURE);
	}

	if (!kexe_load(filename)) {
		return (EXIT_FAILURE);
	}

	vm_init_builtin_classes();
	vm_init_parent_pointers();
	vm_init_const_table_elems();

	stack_init();
	// Call static initializers of all classes
	// or we could do some sort of lazy loading.
	vm_call_class_initializers();
	vm_call_main(argc - optind, argv + optind);

	free_globals();

	if (!mem_free()) {
		fprintf(stderr, "Memory free has failed.\n");
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
