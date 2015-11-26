/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <setjmp.h>

#include "vm.h"
#include "loader.h"
#include "k_array.h"
#include "k_string.h"
#include "k_integer.h"
#include "k_file.h"
#include "k_term.h"
#include "k_sys.h"
#include "k_exception.h"
#include "stack.h"
#include "memory.h"

/******************************************************************************/
/* global variables. (their extern is in vm.h) */

uint32_t classes_cnt_g = 0;
class_t *classes_g = NULL;

size_t const_table_cnt_g = 0;
uint8_t *const_table_g = NULL;

size_t bc_arr_cnt_g = 0;
uint8_t *bc_arr_g = NULL;

jmp_buf bc_loop_env_g;

//class_t *classes_g;
/******************************************************************************/
/* debugging/printing code */

void vm_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);

#ifdef EXIT_ON_ERROR
	exit(EXIT_FAILURE);
#endif
}

#if DEBUG

/* this function is not thread safe */
char *kek_obj_print(kek_obj_t *kek_obj) {
	static char str[1024];

	if (kek_obj == (kek_obj_t *) 0xffffffffffffffff) {
		(void) snprintf(str, 1024, "kek_obj == 0xffffffffffffffff");
		goto out;
	}

	if (kek_obj == NULL) {
		(void) snprintf(str, 1024, "kek_obj == NULL");
		goto out;
	}

	/* vm_debug(DBG_STACK | DBG_STACK_FULL, "kek_obj = %p\n", kek_obj); */
	if (!IS_PTR(kek_obj)) {
		if (IS_CHAR(kek_obj)) {
			(void) snprintf(str, 1024, "char -%c-", CHAR_VAL(kek_obj));
		}
		else if (IS_INT(kek_obj)) {
			(void) snprintf(str, 1024, "int -%d-", INT_VAL(kek_obj));
		}
	}
	else {
		switch (kek_obj->h.t) {
		case KEK_INT:
			(void) snprintf(str, 1024, "int -%d-", INT_VAL(kek_obj));
			break;
		case KEK_STR:
			(void) snprintf(str, 1024, "str -%s-",
					((kek_string_t *) kek_obj)->string);
			break;
		case KEK_ARR:
			(void) snprintf(str, 1024, "arr -%p-", (void*)kek_obj);
			break;
		case KEK_SYM:
			(void) snprintf(str, 1024, "sym -%s-",
					((kek_symbol_t *) kek_obj)->symbol);
			break;
		case KEK_NIL:
			(void) snprintf(str, 1024, "nil");
			break;
		case KEK_UDO:
			(void) snprintf(str, 1024, "udo");
			break;
		default:
			(void) snprintf(str, 1024, "unknown type %d", kek_obj->h.t);
			/* vm_error("kek_obj_print: unhandled type %d\n", kek_obj->type);
			 assert(0 && "unhandled kek_obj->type"); */
			break;
		}
	}

	out: /* */
	return ((char *) (&str));
}

static char *vm_debug_flag(uint32_t flag) {
	switch (flag) {
	case DBG_LOADING:
		return "loading";
	case DBG_BC:
		return "bc";
	case DBG_STACK:
		return "stack";
	case DBG_STACK_FULL:
		return "stack_full";
	case DBG_VM:
		return "vm";
	case DBG_GC:
		return "gc";
	default:
		return"unknown";
	}
}

void vm_debug(uint32_t level_flag, const char *format, ...) {
	va_list args;

	if (debug_level_g & level_flag) {
		fprintf(stderr, "@dbg %s: ", vm_debug_flag(level_flag));
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fflush(stderr);
	}
}
#else

char *kek_obj_print(kek_obj_t *kek_obj) {
	return (NULL);
}

void vm_debug(const char *format, ...) {
	((void)0);
}
#endif
/******************************************************************************/

void vm_init_builtin_classes(void) {
	init_kek_array_class();
	init_kek_string_class();
	init_kek_file_class();
	init_kek_term_class();
	init_kek_sys_class();
	init_kek_exception_class();
}

void vm_init_parent_pointers(void) {
	uint32_t i;
	for (i = 0; i < classes_cnt_g; i++) {
		if (!classes_g[i].parent_name
				|| strlen(classes_g[i].parent_name) == 0) {
			classes_g[i].parent = NULL;
		} else {
			classes_g[i].parent = vm_find_class(classes_g[i].parent_name);
			assert(classes_g[i].parent != NULL);
		}
	}
}

// Init class pointers and fix array layout
void vm_init_const_table_elems(void) {
	uint8_t * ptr = const_table_g;
	class_t * str_cls = vm_find_class("String");
	class_t * arr_cls = vm_find_class("Array");
	constant_array_t * c_arr;
	int i;
	kek_obj_t ** elems;

	while (ptr != const_table_g + const_table_cnt_g) {
		kek_obj_t * obj = (kek_obj_t*) ptr;
		switch (obj->h.t) {
		case KEK_NIL:
			ptr += sizeof(kek_nil_t);
			break;
		case KEK_INT:
			ptr += sizeof(kek_int_t);
			break;
		case KEK_STR:
			obj->h.cls = str_cls;
			ptr += sizeof(kek_string_t) + obj->k_str.length;
			break;
		case KEK_SYM:
			ptr += sizeof(kek_symbol_t) + obj->k_sym.length;
			break;
		case KEK_ARR:
			obj->h.cls = arr_cls;

			c_arr = (constant_array_t*) obj;
			elems = alloc_const_arr_elems(obj->k_arr.length);
			obj->k_arr.alloc_size = obj->k_arr.length;

			for (i = 0; i < c_arr->length; ++i) {
				elems[i] = CONST(c_arr->elems[i]);
			}
			obj->k_arr.elems = elems;

			ptr += sizeof(constant_array_t) + (obj->k_arr.length - 1) * sizeof(uint32_t);
			break;
		case KEK_EXINFO:
			ptr += sizeof(kek_exinfo_t) + (obj->k_exi.length - 1) * sizeof(try_range_t);
			break;
		default:;
		}
	}
}

void vm_init_native_method(method_t * mth, const char * name, uint32_t args_cnt,
		uint8_t is_static, method_ptr func) {
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

method_t * vm_find_method_in_class(class_t * cls, const char * name,
		bool is_static) {
	uint32_t i;

	if (cls->constructor && !strcmp(cls->constructor->name, name)) {
		return cls->constructor;
	}
	for (i = 0; i < cls->methods_cnt; ++i) {
		if (!strcmp(cls->methods[i].name, name)
				&& cls->methods[i].is_static == is_static) {
			return &cls->methods[i];
		}
	}
	if (cls->parent) {
		return vm_find_method_in_class(cls->parent, name, is_static);
	}
	return NULL;
}

symbol_t * vm_find_static_sym_in_class(class_t * cls, const char * name) {
	uint32_t i;

	for (i = 0; i < cls->syms_static_cnt; ++i) {
		if (!strcmp(cls->syms_static[i].name, name)) {
			return &cls->syms_static[i];
		}
	}
	if (cls->parent) {
		return vm_find_static_sym_in_class(cls->parent, name);
	}
	return NULL;
}

symbol_t * vm_find_instance_sym_in_class(class_t * cls, const char * name) {
	uint32_t i;

	for (i = 0; i < cls->syms_instance_cnt; ++i) {
		if (!strcmp(cls->syms_instance[i].name, name)) {
			return &cls->syms_instance[i];
		}
	}
	if (cls->parent) {
		return vm_find_instance_sym_in_class(cls->parent, name);
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

void vm_call_class_initializers(void) {
	uint32_t i;
	for (i = 0; i < classes_cnt_g; ++i) {
		if (classes_g[i].static_init) {
			method_t * init = classes_g[i].static_init;
			if (init->args_cnt != 0) {
				vm_error(
						"Static class initializer should have no arguments.\n");
			}
			vm_debug(DBG_BC, "Entering static class initializer of %s.\n", classes_g[i].name);
			PUSH(&classes_g[i]);
			BC_CALL(init->entry.bc_addr, NATIVE, 0, init->locals_cnt);
			vm_execute_bc();
		}
	}
}

void vm_call_main(int argc, char *argv[]) {
	int i;
	class_t * entry_cls;
	method_t * kek_main;
	kek_array_t * kek_argv;

	// Wrap argv in kek array
	kek_argv = (kek_array_t*) alloc_array(vm_find_class("Array"));
	native_new_array(kek_argv);

	for (i = 0; i < argc; ++i) {
		kek_obj_t * kek_str = new_string_from_cstring(argv[i]);
		native_arr_elem_set(kek_argv, i, kek_str);
	}

	// Locate method main
	kek_main = vm_find_method("main", true, &entry_cls);
	if (kek_main == NULL) {
		vm_error("Cannot find the main method.\n");
	}
	if (kek_main->args_cnt != 1) {
		vm_error("Method main should have one argument.\n");
	}
	vm_debug(DBG_VM, "found %s.%s, entry_point: %u\n", entry_cls->name,
			kek_main->name, kek_main->entry.bc_addr);

	// push argument and class reference
	PUSH(kek_argv);
	PUSH(entry_cls);

	// prepare stack and instruction pointer
	BC_CALL(kek_main->entry.bc_addr, NATIVE, 1, kek_main->locals_cnt);

	// call the bytecode interpreter
	vm_execute_bc();
}

/*const char *op_str[] = { "NOP", "BOP", "UNM", "DR", "ST", "IFNJ", "JU", "WRT",
 "RD", "DUP", "SWAP", "NOT", "STOP", "RET", "CALL", "CALLS", "CALLE",
 "LVBI_C", "LVBI_ARG", "LVBI_LOC", "LVBI_IV", "LVBI_CV", "LVBI_CVE",
 "LVBS_IVE", "LVBS_CVE", "LD_SELF", "LD_CLASS", "LABI_ARG", "LABI_LOC",
 "LABI_IV", "LABI_CV", "LABI_CVE", "LABS_IVE", "LABS_IVE", "IDX", "IDXA",
 "NEW" };*/

const char *bop_str[] = { "Plus", "Minus", "Times", "Divide", "Modulo", "Eq",
		"NotEq", "Less", "Greater", "LessOrEq", "GreaterOrEq", "LogOr",
		"LogAnd", "BitOr", "BitAnd", "Xor", "Lsh", "Rsh", "Error" };

const char *type_str[] = { "KEK_NIL", "KEK_INT", "KEK_STR", "KEK_SYM",
		"KEK_ARR", "KEK_UDO", "KEK_CLASS" };

const char *call_str[] = { "CALLE", "CALL", "CALLS" };

static inline kek_obj_t * bc_bop(op_t o, kek_obj_t *a, kek_obj_t *b) {
	char * str_a, * str_b;
	char chr_a[2], chr_b[2];
	chr_a[1] = chr_b[1] = '\0';

	if (IS_INT(a) && IS_INT(b)) {
		kek_int_t *res;

		vm_debug(DBG_BC, " - %d, %d", INT_VAL(a), INT_VAL(b));

		switch (o) {
		case Plus:
			res = make_integer(INT_VAL(a) + INT_VAL(b));
			break;
		case Minus:
			res = make_integer(INT_VAL(a) - INT_VAL(b));
			break;
		case Times:
			res = make_integer(INT_VAL(a) * INT_VAL(b));
			break;
		case Divide:
			res = make_integer(INT_VAL(a) / INT_VAL(b));
			break;
		case Modulo:
			res = make_integer(INT_VAL(a) % INT_VAL(b));
			break;
		case Eq:
			res = make_integer(INT_VAL(a) == INT_VAL(b));
			break;
		case NotEq:
			res = make_integer(INT_VAL(a) != INT_VAL(b));
			break;
		case Less:
			res = make_integer(INT_VAL(a) < INT_VAL(b));
			break;
		case Greater:
			res = make_integer(INT_VAL(a) > INT_VAL(b));
			break;
		case LessOrEq:
			res = make_integer(INT_VAL(a) <= INT_VAL(b));
			break;
		case GreaterOrEq:
			res = make_integer(INT_VAL(a) >= INT_VAL(b));
			break;
		case LogOr:
			res = make_integer(INT_VAL(a) || INT_VAL(b));
			break;
		case LogAnd:
			res = make_integer(INT_VAL(a) && INT_VAL(b));
			break;
		case BitOr:
			res = make_integer(INT_VAL(a) | INT_VAL(b));
			break;
		case BitAnd:
			res = make_integer(INT_VAL(a) & INT_VAL(b));
			break;
		case Xor:
			res = make_integer(INT_VAL(a) ^ INT_VAL(b));
			break;
		case Lsh:
			res = make_integer(INT_VAL(a) << INT_VAL(b));
			break;
		case Rsh:
			res = make_integer(INT_VAL(a) >> INT_VAL(b));
			break;
		default:
			vm_error("bc_bop: unsupported bop %d on integers\n", o);
			break;
		}
		vm_debug(DBG_BC, " = %d\n", INT_VAL((kek_obj_t* )res));
		return (kek_obj_t*) res;
	}
	else if(( // This is intentionally complicated :D
			(IS_CHAR(a) && (chr_a[0] = CHAR_VAL(a)) && (str_a = chr_a))
			|| (IS_STR(a) && (str_a = a->k_str.string))
			) && (
			(IS_CHAR(b) && (chr_b[0] = CHAR_VAL(b)) && (str_b = chr_b))
			|| (IS_STR(b) && (str_b = b->k_str.string)))) {
		/* After the condition is evaluated, we know this is a str/str
		 * char/str or str/char comparison. Furthermore, we have set up
		 * str_a and str_b to point to the string/char values. */
		kek_obj_t *res;

		switch (o) {
		case Plus:
			res = new_string_from_concat(str_a, str_b);
			break;
		case Eq:
			res = (kek_obj_t*)make_integer(!strcmp(str_a, str_b));
			break;
		case NotEq:
			res = (kek_obj_t*)make_integer(strcmp(str_a, str_b));
			break;
		case Less:
			res = (kek_obj_t*)make_integer(strcmp(str_a, str_b) < 0);
			break;
		case Greater:
			res = (kek_obj_t*)make_integer(strcmp(str_a, str_b) > 0);
			break;
		case LessOrEq:
			res = (kek_obj_t*)make_integer(strcmp(str_a, str_b) <= 0);
			break;
		case GreaterOrEq:
			res = (kek_obj_t*)make_integer(strcmp(str_a, str_b) >= 0);
			break;
		// TODO: implement more operators
		default:
			vm_error("bc_bop: unsupported bop %d on chars/strings\n", o);
			break;
		}
		return res;
	} else {
		kek_int_t *res;

		switch (o) {
		case Eq:
			res = make_integer(a == b);
			break;
		case NotEq:
			res = make_integer(a != b);
			break;
		default:
		vm_error("bc_bop: unsupported bop %d\n", o);
		break;
		}
		return (kek_obj_t*) res;
	}
	return NULL;
}

static inline symbol_t * SYM_STATIC(class_t * cls, int idx) {
	int c_idx = idx - cls->syms_static[0].addr;
	class_t * cls_ptr = cls;

	while(cls_ptr->parent && c_idx < 0) {
		cls_ptr = cls_ptr->parent;
		c_idx = idx - cls_ptr->syms_static[0].addr;
	}

	if (c_idx < 0) {
		vm_error("Class member not found.\n");
	}

	return &cls_ptr->syms_static[c_idx];
}

void vm_execute_bc(void) {
	bc_t op_c;
	kek_obj_t *obj, *idx, **addr, *sym;
	class_t * cls;
	method_t * mth;
	uint16_t arg1, arg2;
	int tick;
	enum {
		E, I, S
	} call_type;

	setjmp(bc_loop_env_g);

	for (tick = 0;; tick++) {
		call_type = -1;
		op_c = bc_arr_g[ip_g];
		decode_instr: // For debugging (gdb can set a breakpoint at label)
		switch (op_c) {
		case LVBI_C: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_C", arg1);

			PUSH(CONST(arg1));
			break;
		}
		case LVBI_ARG: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_ARG", arg1);
			PUSH(ARG(arg1));
			break;
		}
		case LVBI_LOC: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_LOC", arg1);
			PUSH(LOC(arg1));
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(LOC(arg1)));
			break;
		}
		case LABI_ARG: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_ARG", arg1);
			PUSH(&ARG(arg1));
			break;
		}
		case LABI_LOC: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_LOC", arg1);
			PUSH(&LOC(arg1));
			break;
		}
		case ST: {
			vm_debug(DBG_BC, "%s\n", "ST");
			ip_g++;
			POP(obj);
			POP(addr);
			vm_debug(DBG_BC, " - %p = %s\n", addr, kek_obj_print(obj));
			*addr = obj;
			break;
		}
		case IDX: {
			vm_debug(DBG_BC, "%s\n", "IDX");
			ip_g++;
			POP(idx);
			POP(obj);

			if (!obj) {
				vm_error("Invalid pointer.\n");
			}

			if (!IS_INT(idx)) {
				vm_error("Invalid index.\n");
			}
			int idx_n = INT_VAL(idx);

			if (IS_ARR(obj)) {
				if (idx_n < obj->k_arr.length) {
					PUSH(obj->k_arr.elems[idx_n]);
					vm_debug(DBG_BC, " - %s\n", kek_obj_print(obj->k_arr.elems[idx_n]));
				} else {
					vm_error(
							"Array index (%d) out of bounds. Array length is %d\n",
							idx_n, obj->k_arr.length);
				}
			} else if (IS_STR(obj)) {
				if (idx_n < obj->k_str.length) {
					PUSH(MAKE_CHAR(obj->k_str.string[idx_n]));
				} else {
					vm_error(
							"String index (%d) out of bounds. String length is %d\n",
							idx_n, obj->k_str.length);
				}
			} else {
				vm_error("Cannot access this object elements by index.\n");
			}
			break;
		}
		case IDXA: {
			vm_debug(DBG_BC, "%s\n", "IDXA");
			ip_g++;
			POP(idx);
			POP(obj);

			if (obj && IS_ARR(obj) && IS_INT(idx)) {
				int idx_n = INT_VAL(idx);
				if (idx_n >= obj->k_arr.alloc_size) {
					native_grow_array(&obj->k_arr, idx_n + 1);
				} else if (idx_n >= obj->k_arr.length) {
					obj->k_arr.length = idx_n + 1;
				}
				PUSH(&obj->k_arr.elems[idx_n]);
			} else {
				vm_error("Invalid object or index.\n");
			}
			break;
		}
		case BOP: {
			arg1 = BC_OP8(++ip_g);
			ip_g++;
			vm_debug(DBG_BC, "%s %s\n", "BOP", bop_str[arg1]);
			kek_obj_t *op1, *op2, *res;
			POP(op2);
			POP(op1);
			res = bc_bop(arg1, op1, op2);
			PUSH(res);
			break;
		}
		case UNM: {
			vm_debug(DBG_BC, "%s\n", "UNM");
			ip_g++;
			TOP(obj);
			if (IS_INT(obj)) {
				kek_int_t *n = make_integer(-INT_VAL(obj));
				stack_g[sp_g - 1] = (kek_obj_t*) n;
			} else {
				vm_error("Unary minus expects an integer.\n");
			}
			break;
		}
		case NOT: {
			kek_int_t *n;
			vm_debug(DBG_BC, "%s\n", "NOT");
			ip_g++;
			TOP(obj);
			if ((IS_INT(obj) && !INT_VAL(obj))
					|| IS_NIL(obj)) {
				n = make_integer(1);
			} else {
				n = make_integer(0);
			}
			stack_g[sp_g - 1] = (kek_obj_t*) n;

			break;
		}
		case DUP: {
			ip_g++;
			vm_debug(DBG_BC, "%s\n", "DUP");
			PUSH(stack_top());
			break;
		}
		case DR: {
			ip_g++;
			vm_debug(DBG_BC, "%s\n", "DR");
			stack_g[sp_g - 1] = *(kek_obj_t**) stack_top();
			break;
		}
		case WRT: {
			vm_debug(DBG_BC, "%s\n", "WRT");
			ip_g++;

			POP(obj);
			if(IS_CHAR(obj)) {
				printf("%c\n", CHAR_VAL(obj));
			} else if (IS_INT(obj)) {
				printf("%d\n", INT_VAL(obj));
			} else if (IS_STR(obj)) {
				printf("%s", obj->k_str.string);
			} else {
				vm_error(
						"Cannot write object which is not string or integer.\n");
			}

			break;
		}
		case JU: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "JU", arg1);
			ip_g = arg1;
			break;
		}
		case IFNJ: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "IFNJ", arg1);
			POP(obj);
			if (IS_INT(obj)) {
				if (!INT_VAL(obj)) {
					ip_g = arg1;
				}
			} else if (IS_NIL(obj)) { // Jump if false or nil
				ip_g = arg1;
			} else {
				vm_error("Expected integer as the evaluated condition.\n");
			}

			break;
		}
		case CALLS: {
			call_type++;
			// Here is an intentional fallthrough to CALL
		}
		case CALL: {
			call_type++;
			PUSH(THIS);
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(stack_top()));
			// Here is an intentional fallthrough to CALLE
		}
		case CALLE: {
			bool static_call;
			bool tail_call;
			calle: call_type++;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			arg2 = BC_OP16(ip_g);
			ip_g += 2;
			tail_call = bc_arr_g[ip_g] == RET;
			vm_debug(DBG_BC, "%s %u %u, tail: %s\n", call_str[call_type], arg1, arg2, (tail_call ? "true" : "false"));
			TOP(obj);

			if (!IS_PTR(obj)) {
				vm_error("Invalid class/object pointer.\n");
			}

			if (IS_CLASS(obj)) { // Call of static method
				vm_debug(DBG_BC, " - Static method call\n");
				cls = (class_t*) obj;
				static_call = true;
			} else { // Call of instance method
				cls = obj->h.cls;
				static_call = false;

				if (!cls) {
					vm_error("Primitive object does not have any methods.\n");
				}
				vm_debug(DBG_BC, " - Instance method call\n");
			}

			if (call_type == S) {
				cls = cls->parent;
			}

			sym = CONST(arg1); // name of the method
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the first argument of CALL.\n");
			}
			assert(cls);
			mth = vm_find_method_in_class(cls, sym->k_sym.symbol, static_call);
			if (mth == NULL) {
				vm_error("%s has no method %s.\n",
						(static_call ? "Class" : "Object"), sym->k_sym.symbol);
			}
			vm_debug(DBG_BC, " - method name: %s\n", sym->k_sym.symbol);
			if (mth->args_cnt != arg2) {
				vm_error("Method expects %d arguments, %d given.\n", mth->args_cnt, arg2);
			}
			if (mth->is_native) {
				if (tail_call) {
					BC_TCALL(NATIVE, mth->args_cnt, mth->locals_cnt);
				} else {
					BC_CALL(NATIVE, ip_g, mth->args_cnt, mth->locals_cnt);
				}
				mth->entry.func();
				vm_debug(DBG_BC, " - returned value is %s\n", kek_obj_print(stack_top()));
			} else {
				if (tail_call) {
					vm_debug(DBG_BC, " - tail call executed\n");
					BC_TCALL(mth->entry.bc_addr, mth->args_cnt, mth->locals_cnt);
				} else {
					BC_CALL(mth->entry.bc_addr, ip_g, mth->args_cnt,
							mth->locals_cnt);
				}
			}

			break;
		}
		case RET: {
			vm_debug(DBG_BC, "%s\n", "RET");
			BC_RET;
			//vm_debug("ret_addr = %d\n", ip_g);
			if (ip_g == NATIVE) {
				return;
			}
			break;
		}
		case RET_SELF: {
			vm_debug(DBG_BC, "%s\n", "RET_SELF");
			BC_RET_SELF;
			//vm_debug("ret_addr = %d\n", ip_g);
			if (ip_g == NATIVE) {
				return;
			}
			break;
		}
		case LABI_CV: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_CV", arg1);
			obj = THIS;
			if (IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				cls = obj->h.cls;
			}
			cls_memb = SYM_STATIC(cls, arg1);
			vm_debug(DBG_BC, " - load address of static symbol \"%s\"\n",
					cls_memb->name);
			if (cls_memb->const_flag) {
				vm_error("Lvalue cannot be a constant.\n");
			}
			PUSH(&cls_memb->value);
			break;
		}
		case LABI_CVE: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_CVE", arg1);
			POP(obj);
			if (IS_PTR(obj) && IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				vm_error("Expected class pointer on the stack.\n");
			}
			cls_memb = SYM_STATIC(cls, arg1);
			vm_debug(DBG_BC, " - load address of static symbol \"%s\"\n",
					cls_memb->name);
			if (cls_memb->const_flag) {
				vm_error("Lvalue cannot be a constant.\n");
			}
			PUSH(&cls_memb->value);
			break;
		}
		case LVBI_CV: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_CV", arg1);
			obj = THIS;
			if (IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				cls = obj->h.cls;
			}
			cls_memb = SYM_STATIC(cls, arg1);
			vm_debug(DBG_BC, " - load value of static symbol \"%s\"\n",
					cls_memb->name);
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(cls_memb->value));
			PUSH(cls_memb->value);
			break;
		}
		case LVBI_CVE: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_CVE", arg1);
			POP(obj);
			if (IS_PTR(obj) && IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				vm_error("Expected class pointer on the stack.\n");
			}
			cls_memb = SYM_STATIC(cls, arg1);
			vm_debug(DBG_BC, " - load value of static symbol \"%s\"\n",
					cls_memb->name);
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(cls_memb->value));
			PUSH(cls_memb->value);
			break;
		}
		case LVBS_CVE: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBS_CVE", arg1);
			POP(obj);
			if (IS_PTR(obj) && IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				vm_error("Expected class pointer on the stack.\n");
			}
			sym = CONST(arg1); // name of the static member
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of LVBS_CVE.\n");
			}
			cls_memb = vm_find_static_sym_in_class(cls, sym->k_sym.symbol);
			if (!cls_memb) {
				vm_error("Static class member %s not found in %s.\n", sym->k_sym.symbol, cls->name);
			}
			vm_debug(DBG_BC, " - load value of static symbol \"%s\"\n",
					cls_memb->name);
			PUSH(cls_memb->value);
			break;
		}
		case LABS_CVE: {
			symbol_t * cls_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBS_CVE", arg1);
			POP(obj);
			if (IS_PTR(obj) && IS_CLASS(obj)) {
				cls = (class_t*) obj;
			} else {
				vm_error("Expected class pointer on the stack.\n");
			}
			sym = CONST(arg1); // name of the static member
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of LVBS_CVE.\n");
			}
			cls_memb = vm_find_static_sym_in_class(cls, sym->k_sym.symbol);
			if (!cls_memb) {
				vm_error("Static class member %s not found in %s.\n", sym->k_sym.symbol, cls->name);
			}
			vm_debug(DBG_BC, " - load value of static symbol \"%s\"\n",
					cls_memb->name);
			if (cls_memb->const_flag) {
				vm_error("Lvalue cannot be a constant.\n");
			}
			PUSH(&cls_memb->value);
			break;
		}
		case NEW: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			arg2 = BC_OP16(ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u %u\n", "NEW", arg1, arg2);
			sym = CONST(arg1);
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of NEW.\n");
			}
			cls = vm_find_class(sym->k_sym.symbol);
			if (!cls) {
				vm_error("Cannot find class %s.\n", sym->k_sym.symbol);
			}
			obj = cls->allocator(cls); // Allocate tagged memory for the object
			mth = cls->constructor;

			PUSH(obj); // Push instance pointer (THIS)
			if (!mth) { // no constructor	
				break;
			}
			if (mth->args_cnt != arg2) {
				vm_error("Constructor expects %d arguments, %d given.\n", mth->args_cnt, arg2);
			}
			// Call constructor
			if (mth->is_native) {
				BC_CALL(NATIVE, ip_g, mth->args_cnt, mth->locals_cnt);
				mth->entry.func();
			} else {
				BC_CALL(mth->entry.bc_addr, ip_g, mth->args_cnt,
						mth->locals_cnt);
			}
			break;
		}
		case LD_SELF: {
			ip_g++;
			vm_debug(DBG_BC, "%s\n", "LD_SELF");
			PUSH(THIS);
			break;
		}
		case LD_CLASS: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LD_CLASS", arg1);
			sym = CONST(arg1);
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of LD_CLASS.\n");
			}
			cls = vm_find_class(sym->k_sym.symbol);
			if (!cls) {
				vm_error("Cannot find class %s.\n", sym->k_sym.symbol);
			}
			PUSH(cls);
			break;
		}
		case LABI_IV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_IV", arg1);
			obj = THIS;
			assert(IS_UDO(obj));
			PUSH(&INST_VAR(obj, arg1));
			break;
		}
		case LVBI_IV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_IV", arg1);
			obj = THIS;
			assert(IS_UDO(obj));
			PUSH(INST_VAR(obj, arg1));
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(INST_VAR(obj, arg1)));
			break;
		}
		case LVBS_IVE: {
			symbol_t * obj_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBS_IVE", arg1);
			POP(obj);
			if (!IS_PTR(obj)) {
				vm_error("Invalid object pointer.\n");
			}
			assert(!IS_CLASS(obj));
			sym = CONST(arg1);
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of LVBS_IVE.\n");
			}
			if (!obj->h.cls) {
				vm_error("Primitive object does not have any members.\n");
			}
			obj_memb = vm_find_instance_sym_in_class(obj->h.cls, sym->k_sym.symbol);
			if (!obj_memb) {
				vm_error("Instance member %s not found in %s.\n", sym->k_sym.symbol, obj->h.cls->name);
			}
			PUSH(INST_VAR(obj, obj_memb->addr));
			break;
		}
		case LABS_IVE: {
			symbol_t * obj_memb;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABS_IVE", arg1);
			POP(obj);
			if (!IS_PTR(obj)) {
				vm_error("Invalid object pointer.\n");
			}
			assert(!IS_CLASS(obj));
			sym = CONST(arg1);
			if (!IS_SYM(sym)) {
				vm_error("Expected symbol as the argument of LABS_IVE.\n");
			}
			if (!obj->h.cls) {
				vm_error("Primitive object does not have any members.\n");
			}
			obj_memb = vm_find_instance_sym_in_class(obj->h.cls, sym->k_sym.symbol);
			if (!obj_memb) {
				vm_error("Instance member %s not found in %s.\n", sym->k_sym.symbol, obj->h.cls->name);
			}
			PUSH(&INST_VAR(obj, obj_memb->addr));
			break;
		}
		case ST_EXINFO: {
			kek_obj_t * exi;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "ST_EXINFO", arg1);
			exi = CONST(arg1);
			assert(exi->h.t == KEK_EXINFO);
			stack_g[fp_g] = exi;
			break;
		}
		case THROW: {
			ip_g++;
			vm_debug(DBG_BC, "%s\n", "THROW");
			POP(obj);
			vm_throw_obj(obj);
			break;
		}
		case LD_EXOBJ: {
			ip_g++;
			vm_debug(DBG_BC, "%s\n", "LD_EXOBJ");
			assert(stack_g[fp_g]->h.t == KEK_EXINFO);
			PUSH(stack_g[fp_g]->k_exi.obj_thrown);
			break;
		}
		default:
			vm_error("Invalid instruction at %u\n", ip_g);
			break;
		} /* switch */

		if (tick >= gc_ticks_g) {
			tick = 0;
			gc();
		}
	} /* for */
} /* vm_execute_bc */

#define UNWIND() { \
	sp = ap; \
	ret_addr = (size_t)INT_VAL(stack_g[fp - 3]); \
	ap = (size_t)INT_VAL(stack_g[fp - 2]); \
	fp = (size_t)INT_VAL(stack_g[fp - 1]); \
	if (ret_addr == NATIVE) return -1; \
}

static int find_handler_for_exobj(kek_obj_t * obj,
		int * unw_ap, int * unw_fp, int * unw_sp) {
	int ap = ap_g;
	int fp = fp_g;
	int sp = sp_g;
	int ret_addr = ip_g + 1; // Right after THROW
	kek_exinfo_t * exi;
	int i;

	while (true) {
		// Unwind stack
		while (stack_g[fp] == NULL) {
			UNWIND();
		}

		// Check if we are inside a try block
		exi = &stack_g[fp]->k_exi;
		for (i = 0; i < exi->length; ++i) {
			if (exi->blocks[i].try_addr <= (ret_addr - 1)
					&& (ret_addr - 1) < exi->blocks[i].catch_addr) {
				exi->obj_thrown = obj;
				*unw_ap = ap;
				*unw_fp = fp;
				*unw_sp = sp;
				return exi->blocks[i].catch_addr;
			}
		}

		// If not, continue unwinding
		UNWIND();
	}

	return -1;
}

void vm_throw_obj(kek_obj_t * obj) {
	int unw_ap;
	int unw_fp;
	int unw_sp;
	int unw_ip;
	unw_ip = find_handler_for_exobj(obj, &unw_ap, &unw_fp, &unw_sp);
	if (unw_ip == -1) {
		if (IS_PTR(obj) && obj->h.cls) {
			vm_error("Unhandled exception %s.\n", obj->h.cls->name);
		} else {
			vm_error("Unhandled exception %s.\n", kek_obj_print(obj));
		}
	}
	ap_g = unw_ap;
	fp_g = unw_fp;
	sp_g = unw_sp;
	ip_g = unw_ip;
}

void vm_throw_obj_from_native_ctxt(kek_obj_t * obj) {
	vm_throw_obj(obj);
	longjmp(bc_loop_env_g, 1);
}
