/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "array.h"

void init_kek_array_class(void) {
	char name[] = "Array";
	classes_g[classes_cnt_g].name = malloc(strlen(name + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 0;
	// TODO: add native methods such as length(), get_idx(), get_idxa()
	classes_g[classes_cnt_g].methods = NULL;
	// TODO: add allocator
	// TODO: add constructor of empty Array
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}