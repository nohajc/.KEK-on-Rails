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
#include "k_file.h"
#include "k_string.h"
#include "stack.h"

void init_kek_file_class(void) {
	char name[] = "File";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 3;
	classes_g[classes_cnt_g].methods = malloc(
		classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "close", 0, false, file_close);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[1], "readln", 0, false, file_readln);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[2], "writeln", 1, false, file_readln);

	classes_g[classes_cnt_g].allocator = alloc_file;

	classes_g[classes_cnt_g].constructor = malloc(sizeof(method_t));
	vm_init_native_method(classes_g[classes_cnt_g].constructor, "File", 2, false, new_file);
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

void new_file(void) {
	kek_file_t * fil = (kek_file_t*)THIS;
	kek_obj_t * path = ARG(0);
	kek_obj_t * mode = ARG(1);

	if (!IS_STR(path) || !IS_STR(mode)) {
		vm_error("File constructor expects two string arguments.\n");
	}

	fil->f_handle = fopen(path->k_str.string, mode->k_str.string);
	vm_debug(DBG_BC, "Opened file with handle %p.\n", fil->f_handle);
	// TODO: error handling - we need exceptions

	BC_RETVOID;
}

void file_close(void) {
	kek_file_t * fil = (kek_file_t*)THIS;

	fclose(fil->f_handle);

	PUSH(NIL);
	BC_RET;
}
void file_readln(void) {
	kek_file_t * fil = (kek_file_t*)THIS;
	kek_obj_t * str;

	if (!fil->f_handle) {
		vm_error("Invalid file handle.\n");
	}

	int buf_size = 64;
	int len;
	char * st;
	char * buf = malloc(buf_size * sizeof(char));
	int line_idx = 0;

	while(true) {
		st = fgets(buf + line_idx, buf_size - line_idx, fil->f_handle);
		if (!st) {
			free(buf);
			PUSH(NIL);
			BC_RET;
			return;
		}
		len = strlen(buf + line_idx);
		if(buf[line_idx + len - 1] == '\n') {
			break;
		}
		else {
			buf_size *= 2;
			buf = realloc(buf, buf_size * sizeof(char));
			line_idx += len;
		}
	}

	str = new_string_from_cstring(buf);
	free(buf);
	PUSH(str);
	BC_RET;
}

void file_writeln(void) {
	// TODO: implement
}