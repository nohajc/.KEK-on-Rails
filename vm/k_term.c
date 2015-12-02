/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include "vm.h"
#include "memory.h"
#include "k_term.h"
#include "k_string.h"
#include "stack.h"

void init_kek_term_class(void) {
	char name[] = "Term";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 1;
	classes_g[classes_cnt_g].methods = malloc(
		classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "readln", 1, false, term_readln);

	// Term object has no state - it is just a simple wrapper of C functions
	classes_g[classes_cnt_g].allocator = alloc_term;
	classes_g[classes_cnt_g].constructor = NULL;

	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

void term_readln(void) {
	kek_obj_t * prompt = ARG(0);
	kek_obj_t * str;

	if (!IS_STR(prompt)) {
		vm_error("Expected string as argument.\n");
	}
	char * buf = readline(prompt->k_str.string);
	if (!buf) {
		free(buf);
		PUSH(NIL);
		BC_RET;
		return;
	}
	str = new_string_from_concat(buf, "\n");
	if (buf && *buf) {
		add_history(buf);
	}
	free(buf);
	PUSH(str);
	BC_RET;
}
