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
#include "k_meta.h"
#include "stack.h"

void init_kek_meta_class(void) {
	char name[] = "Meta";

	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 1;

	classes_g[classes_cnt_g].methods = malloc(
			classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "_new", 2,
			true, meta_new);

	classes_g[classes_cnt_g].allocator = NULL;
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

void meta_new(void) {
	kek_obj_t * sym = ARG(0);
	kek_obj_t * args = ARG(1);
	kek_obj_t * obj;
	int args_cnt;
	class_t * cls;
	method_t * cons;
	int i;

	if (!IS_SYM(sym)) {
		vm_error("Expected symbol as first argument.\n");
	}

	cls = vm_find_class(sym->k_sym.symbol);
	if (!cls) {
		vm_error("Cannot find class %s.", sym->k_sym.symbol);
	}
	if (!cls->allocator) {
		vm_error("Cannot create instance of %s.", sym->k_sym.symbol);
	}
	obj = cls->allocator(cls);

	cons = cls->constructor;
	if (!cons) {
		args_cnt = 0;
	}
	else {
		args_cnt = cons->args_cnt;
		if (args_cnt > 0 && (!IS_ARR(args) || args->k_arr.length != args_cnt)) {
			vm_error("Constructor expects %d arguments, %d given.\n",
						cons->args_cnt, IS_ARR(args) ? args->k_arr.length : 0);
		}
	}

	for (i = 0; i < args_cnt; i++) {
		PUSH((args->k_arr.elems[i]));
	}
	PUSH(obj);

	if (!cons) { // no constructor
		BC_RET;
		return;
	}

	// Call constructor
	if (cons->is_native) {
		BC_CALL(NATIVE, NATIVE, cons->args_cnt, cons->locals_cnt);
		cons->entry.func();
	} else {
		BC_CALL(cons->entry.bc_addr, NATIVE, cons->args_cnt,
				cons->locals_cnt);
		vm_execute_bc();
	}
	BC_RET;
}
