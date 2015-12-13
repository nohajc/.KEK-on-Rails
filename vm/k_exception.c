/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "vm.h"
#include "memory.h"
#include "k_exception.h"
#include "stack.h"

void init_kek_exception_class(void) {
	char name[] = "Exception";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 2;
	classes_g[classes_cnt_g].methods = malloc(
		classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "msg", 0, false, exception_msg);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[1], "type", 0, false, exception_type);

	classes_g[classes_cnt_g].allocator = alloc_exception;

	classes_g[classes_cnt_g].constructor = malloc(sizeof(method_t));
	vm_init_native_method(classes_g[classes_cnt_g].constructor, "Exception", 1, false, new_exception);

	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

void new_exception(void) {
	kek_except_t * expt = (kek_except_t*)THIS;
	kek_obj_t * msg = ARG(0);
	native_new_exception(expt, msg);

	BC_RET_SELF;
}
void native_new_exception(kek_except_t * expt, kek_obj_t * msg) {
	expt->msg = msg;
}

kek_except_t * make_exception(kek_obj_t * msg) {
	uint32_t id = gc_rootset_add((kek_obj_t **) &msg);
	class_t * expt_class = vm_find_class("Exception");
	kek_except_t * expt = (kek_except_t*) alloc_exception(expt_class);
	native_new_exception(expt, msg);

	gc_rootset_remove_id(id);
	return expt;
}

void exception_msg(void) {
	kek_except_t * expt = (kek_except_t*)THIS;
	PUSH(expt->msg);
	BC_RET;
}

void exception_type(void) {
	kek_except_t * expt = (kek_except_t*)THIS;
	PUSH(expt->h.cls);
	BC_RET;
}
