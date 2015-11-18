/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <stdio.h>
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
	class_t * file_cls = vm_find_class("File");

	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 0;

	classes_g[classes_cnt_g].methods = NULL;

	classes_g[classes_cnt_g].allocator = NULL;
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 3;
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

	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}