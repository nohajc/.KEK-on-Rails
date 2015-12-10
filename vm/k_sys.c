/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "vm.h"
#include "memory.h"
#include "k_sys.h"
#include "k_file.h"
#include "stack.h"

void init_kek_sys_class(void) {
	char name[] = "Sys";
	char m_out[] = "out";
	char m_in[] = "in";
	char m_err[] = "err";
	char m_term[] = "term";
	class_t * file_cls = vm_find_class("File");
	class_t * term_cls = vm_find_class("Term");

	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 3;

	classes_g[classes_cnt_g].methods = malloc(
			classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "rand", 0,
	true, sys_rand);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[1], "srand", 1,
	true, sys_srand);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[2], "time", 0,
	true, sys_time);

	classes_g[classes_cnt_g].allocator = NULL;
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 4;
	classes_g[classes_cnt_g].syms_static = malloc(
		classes_g[classes_cnt_g].syms_static_cnt * sizeof(symbol_t));

	// Save pointer to stdout
	classes_g[classes_cnt_g].syms_static[0].name = malloc((strlen(m_out) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].syms_static[0].name, m_out);
	classes_g[classes_cnt_g].syms_static[0].value = alloc_file(file_cls);
	classes_g[classes_cnt_g].syms_static[0].value->k_fil.f_handle = stdout;
	classes_g[classes_cnt_g].syms_static[0].const_ptr = 0;
	classes_g[classes_cnt_g].syms_static[0].const_flag = CONST;
	classes_g[classes_cnt_g].syms_static[0].addr = 0;

	// Save pointer to stdin
	classes_g[classes_cnt_g].syms_static[1].name = malloc((strlen(m_in) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].syms_static[1].name, m_in);
	classes_g[classes_cnt_g].syms_static[1].value = alloc_file(file_cls);
	classes_g[classes_cnt_g].syms_static[1].value->k_fil.f_handle = stdin;
	classes_g[classes_cnt_g].syms_static[1].const_ptr = 0;
	classes_g[classes_cnt_g].syms_static[1].const_flag = CONST;
	classes_g[classes_cnt_g].syms_static[1].addr = 1;

	// Save pointer to stderr
	classes_g[classes_cnt_g].syms_static[2].name = malloc((strlen(m_err) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].syms_static[2].name, m_err);
	classes_g[classes_cnt_g].syms_static[2].value = alloc_file(file_cls);
	classes_g[classes_cnt_g].syms_static[2].value->k_fil.f_handle = stderr;
	classes_g[classes_cnt_g].syms_static[2].const_ptr = 0;
	classes_g[classes_cnt_g].syms_static[2].const_flag = CONST;
	classes_g[classes_cnt_g].syms_static[2].addr = 2;

	// Save pointer to term
	classes_g[classes_cnt_g].syms_static[3].name = malloc((strlen(m_term) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].syms_static[3].name, m_term);
	classes_g[classes_cnt_g].syms_static[3].value = alloc_term(term_cls);
	classes_g[classes_cnt_g].syms_static[3].const_ptr = 0;
	classes_g[classes_cnt_g].syms_static[3].const_flag = CONST;
	classes_g[classes_cnt_g].syms_static[3].addr = 3;

	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

void sys_rand(void) {
	PUSH(make_integer(rand()));
	BC_RET;
}

void sys_srand(void) {
	kek_obj_t * seed = ARG(0);
	if (!IS_INT(seed)) {
		vm_error("Expected integer as argument.\n");
	}
	srand(INT_VAL(seed));
	PUSH(NIL);
	BC_RET;
}

void sys_time(void) {
	PUSH(make_integer(time(NULL)));
	BC_RET;
}