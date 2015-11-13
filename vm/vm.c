/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

#include "vm.h"
#include "loader.h"
#include "k_array.h"
#include "k_string.h"
#include "stack.h"
#include "memory.h"

/******************************************************************************/
/* global variables */

//class_t *classes_g;

/******************************************************************************/
/* debugging/printing code */

void vm_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}

#if DEBUG
void vm_debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
}
#else
void vm_debug(const char *format, ...) {
}
#endif
/******************************************************************************/

void vm_init_builtin_classes(void) {
	init_kek_array_class();
	init_kek_string_class();
}

void vm_init_native_method(method_t * mth, const char * name, uint32_t args_cnt, uint8_t is_static, method_ptr func) {
	mth->name = malloc(strlen(name) + 1);
	strcpy(mth->name, name);
	mth->entry.func = func;
	mth->args_cnt = args_cnt;
	mth->locals_cnt = 0;
	mth->is_static = is_static;
	mth->is_native = true;
}

class_t * vm_find_class(const char * name) {
	uint32_t i;
	for (i = 0; i < classes_cnt_g; ++i) {
		if (!strcmp(classes_g[i].name, name)) {
			return &classes_g[i];
		}
	}
	return NULL;
}

method_t * vm_find_method_in_class(class_t * cls, const char * name, bool is_static) {
	uint32_t i;

	for (i = 0; i < cls->methods_cnt; ++i) {
		if (!strcmp(cls->methods[i].name, name) && cls->methods[i].is_static == is_static) {
			return &cls->methods[i];
		}
	}
	return NULL;
}

method_t * vm_find_method(const char * name, bool is_static, class_t ** cls) {
	uint32_t i;
	for (i = 0; i < classes_cnt_g; ++i) {
		method_t * m = vm_find_method_in_class(&classes_g[i], name, is_static);
		if (m) {
			*cls = &classes_g[i];
			return m;
		}
	}
	return NULL;
}

void vm_call_main(int argc, char *argv[]) {
	int i;
	class_t * entry_cls;
	method_t * kek_main;
	kek_array_t * kek_argv;

	// Wrap argv in kek array
	kek_argv = (kek_array_t*)alloc_array(vm_find_class("Array"));
	native_new_array(kek_argv);

	for (i = 0; i < argc; ++i) {
		kek_obj_t * kek_str = new_string_from_cstring(argv[i]);
		native_set(kek_argv, i, kek_str);
	}

	// Locate method main
	kek_main = vm_find_method("main", true, &entry_cls);
	vm_debug("found %s.%s, entry_point: %u\n", entry_cls->name, kek_main->name, kek_main->entry.bc_addr);

	stack_init();
	// push argument and class reference
	PUSH(kek_argv);
	PUSH(entry_cls);

	// prepare stack and instruction pointer
	BC_CALL(kek_main->entry.bc_addr, NATIVE, 1, kek_main->locals_cnt);

	// call the bytecode interpreter
	vm_execute_bc();
}

const char *op_str[] = { "NOP", "BOP", "UNM", "DR", "ST", "IFNJ", "JU", "WRT",
		"RD", "DUP", "SWAP", "NOT", "STOP", "RET", "CALL", "CALLS", "CALLE",
		"LVBI_C", "LVBI_ARG", "LVBI_LOC", "LVBI_IV", "LVBI_CV", "LVBI_CVE",
		"LVBS_IVE", "LVBS_CVE", "LD_SELF", "LD_CLASS", "LABI_ARG", "LABI_LOC",
		"LABI_IV", "LABI_CV", "LABI_CVE", "LABS_IVE", "LABS_IVE", "IDX", "IDXA",
		"NEW" };

void vm_execute_bc(void) {
	bc_t op_c;
	kek_obj_t * obj, * idx;
	uint16_t arg1, arg2;

	while (true) {
		op_c = bc_arr_g[ip_g];
		switch (op_c) {
			case LVBI_C: {
				arg1 = *(uint16_t*)&bc_arr_g[++ip_g];
				ip_g += 2;
				vm_debug("%s %u\n", "LVBI_C", arg1);

				PUSH(CONST(arg1));
				break;
			}
			case LVBI_ARG: {
				arg1 = *(uint16_t*)&bc_arr_g[++ip_g];
				ip_g += 2;
				vm_debug("%s %u\n", "LVBI_ARG", arg1);
				PUSH(ARG(arg1));
				break;
			}
			case IDX: {
				vm_debug("%s\n", "IDX");
				ip_g++;
				POP(idx);
				POP(obj);

				if (IS_ARR(obj) && IS_INT(idx)) {
					PUSH(obj->k_arr.elems[idx->k_int.value]);
				}
			}
			case WRT: {
				vm_debug("%s\n", "WRT");
				ip_g++;

				POP(obj);
				if (IS_STR(obj)) {
					printf("%s", obj->k_str.string);
				}

				break;
			}
			case RET: {
				vm_debug("%s\n", "RET");
				BC_RET;
				//vm_debug("ret_addr = %d\n", ip_g);
				if (ip_g == NATIVE) {
					return;
				}
				break;
			}
			default:
				fprintf(stderr, "Invalid instruction at %u\n", ip_g);
				exit(1);
		}
	}
}
