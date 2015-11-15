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
#include "k_integer.h"
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
	switch (kek_obj->type) {
	case KEK_INT:
		(void) snprintf(str, 1024, "int -%d-", ((kek_int_t *) kek_obj)->value);
		break;
	case KEK_STR:
		(void) snprintf(str, 1024, "str -%s-",
				((kek_string_t *) kek_obj)->string);
		break;
	case KEK_ARR:
		(void) snprintf(str, 1024, "arr");
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
		(void) snprintf(str, 1024, "unknown type %d", kek_obj->type);
		/* vm_error("kek_obj_print: unhandled type %d\n", kek_obj->type);
		 assert(0 && "unhandled kek_obj->type"); */
		break;
	}

	out: /* */
	return ((char *) (&str));
}

void vm_debug(uint32_t level_flag, const char *format, ...) {
	va_list args;

	if (debug_level_g & level_flag) {
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

	for (i = 0; i < cls->methods_cnt; ++i) {
		if (!strcmp(cls->methods[i].name, name)
				&& cls->methods[i].is_static == is_static) {
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
	// TODO: search through parent classes if not found in the given class
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
	if (a->h.t == KEK_INT && b->h.t == KEK_INT) {
		kek_int_t *res = (kek_int_t*) alloc_integer();

		vm_debug(DBG_BC, " - %d, %d", INT_VALUE(a), INT_VALUE(b));

		switch (o) {
		case Plus:
			native_new_integer(res, INT_VALUE(a) + INT_VALUE(b));
			break;
		case Minus:
			native_new_integer(res, INT_VALUE(a) - INT_VALUE(b));
			break;
		case Times:
			native_new_integer(res, INT_VALUE(a) * INT_VALUE(b));
			break;
		case Divide:
			native_new_integer(res, INT_VALUE(a) / INT_VALUE(b));
			break;
		case Modulo:
			native_new_integer(res, INT_VALUE(a) % INT_VALUE(b));
			break;
		case Eq:
			native_new_integer(res, INT_VALUE(a) == INT_VALUE(b));
			break;
		case NotEq:
			native_new_integer(res, INT_VALUE(a) != INT_VALUE(b));
			break;
		case Less:
			native_new_integer(res, INT_VALUE(a) < INT_VALUE(b));
			break;
		case Greater:
			native_new_integer(res, INT_VALUE(a) > INT_VALUE(b));
			break;
		case LessOrEq:
			native_new_integer(res, INT_VALUE(a) <= INT_VALUE(b));
			break;
		case GreaterOrEq:
			native_new_integer(res, INT_VALUE(a) >= INT_VALUE(b));
			break;
		case LogOr:
			native_new_integer(res, INT_VALUE(a) || INT_VALUE(b));
			break;
		case LogAnd:
			native_new_integer(res, INT_VALUE(a) && INT_VALUE(b));
			break;
		case BitOr:
			native_new_integer(res, INT_VALUE(a) | INT_VALUE(b));
			break;
		case BitAnd:
			native_new_integer(res, INT_VALUE(a) & INT_VALUE(b));
			break;
		case Xor:
			native_new_integer(res, INT_VALUE(a) ^ INT_VALUE(b));
			break;
		case Lsh:
			native_new_integer(res, INT_VALUE(a) << INT_VALUE(b));
			break;
		case Rsh:
			native_new_integer(res, INT_VALUE(a) >> INT_VALUE(b));
			break;
		default:
			vm_error("bc_bop: unknown bop %d\n", o);
			break;
		}
		vm_debug(DBG_BC, " = %d\n", INT_VALUE((kek_obj_t* )res));
		return (kek_obj_t*) res;
	} else {
		// TODO: implement operations for other types
		vm_error("Cannot apply operation %s to %s and %s.\n", bop_str[o],
				type_str[a->h.t], type_str[b->h.t]);
	}
	return NULL;
}

void vm_execute_bc(void) {
	bc_t op_c;
	kek_obj_t *obj, *idx, **addr, *sym;
	class_t * cls;
	method_t * mth;
	uint16_t arg1, arg2;
	enum {
		E, I, S
	} call_type;

	while (true) {
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
			*addr = obj;
			vm_debug(DBG_BC, " - %s\n", kek_obj_print(obj));
			break;
		}
		case IDX: {
			vm_debug(DBG_BC, "%s\n", "IDX");
			ip_g++;
			POP(idx);
			POP(obj);

			if (IS_ARR(obj) && IS_INT(idx)) {
				int idx_n = INT_VALUE(idx);
				if (idx_n < obj->k_arr.length) {
					PUSH(obj->k_arr.elems[idx_n]);
				} else {
					vm_error(
							"Array index (%d) out of bounds. Array length is %d\n",
							idx_n, obj->k_arr.length);
				}
			} else {
				vm_error("Invalid array or index.\n");
			}
			break;
		}
		case IDXA: {
			vm_debug(DBG_BC, "%s\n", "IDXA");
			ip_g++;
			POP(idx);
			POP(obj);

			if (IS_ARR(obj) && IS_INT(idx)) {
				int idx_n = INT_VALUE(idx);
				if (idx_n >= obj->k_arr.alloc_size) {
					native_grow_array(&obj->k_arr, idx_n + 1);
				} else if (idx_n >= obj->k_arr.length) {
					obj->k_arr.length = idx_n + 1;
				}
				PUSH(&obj->k_arr.elems[INT_VALUE(idx)]);
			} else {
				vm_error("Invalid array or index.\n");
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
				kek_int_t *n = (kek_int_t*) alloc_integer();
				native_new_integer(n, -obj->k_int.value);
				stack_g[sp_g - 1] = (kek_obj_t*) n;
			} else {
				vm_error("Unary minus expects an integer.\n");
			}
			break;
		}
		case NOT: {
			kek_int_t *n = (kek_int_t*) alloc_integer();
			vm_debug(DBG_BC, "%s\n", "NOT");
			ip_g++;
			TOP(obj);
			if ((obj->h.t == KEK_INT && !INT_VALUE(obj))
					|| obj->h.t == KEK_NIL) {
				native_new_integer(n, 1);
			} else {
				native_new_integer(n, 0);
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
			if (IS_STR(obj)) {
				printf("%s", obj->k_str.string);
			} else if (IS_INT(obj)) {
				printf("%d\n", obj->k_int.value);
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
			if (obj->h.t == KEK_INT) {
				if (!INT_VALUE(obj)) {
					ip_g = arg1;
				}
			} else if (obj->h.t == KEK_NIL) { // Jump if false or nil
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
			calle: call_type++;
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			arg2 = BC_OP16(ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u %u\n", call_str[call_type], arg1, arg2);
			TOP(obj);

			if (obj->h.t == KEK_CLASS) { // Call of static method
				vm_debug(DBG_BC, " - Static method call\n");
				cls = (class_t*) obj;
				static_call = true;
			} else { // Call of instance method
				vm_debug(DBG_BC, " - Instance method call\n");
				cls = obj->h.cls;
				static_call = false;
			}

			if (call_type == S) {
				cls = cls->parent;
			}

			sym = CONST(arg1); // name of the method
			if (sym->h.t != KEK_SYM) {
				vm_error("Expected symbol as the first argument of CALL.\n");
			}
			assert(cls);
			mth = vm_find_method_in_class(cls, sym->k_sym.symbol, static_call);
			if (mth == NULL) {
				vm_error("%s has no method %s.\n",
						(static_call ? "Class" : "Object"), sym->k_sym.symbol);
			}
			if (mth->args_cnt != arg2) {
				vm_error("Method expects %d arguments, %d given.\n", mth->args_cnt, arg2);
			}
			if (mth->is_native) {
				BC_CALL(NATIVE, ip_g, mth->args_cnt, mth->locals_cnt);
				mth->entry.func();
				vm_debug(DBG_BC, " - returned value is %s\n", kek_obj_print(stack_top()));
			} else {
				BC_CALL(mth->entry.bc_addr, ip_g, mth->args_cnt,
						mth->locals_cnt);
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
		case RETVOID: {
			vm_debug(DBG_BC, "%s\n", "RETVOID");
			BC_RETVOID;
			//vm_debug("ret_addr = %d\n", ip_g);
			if (ip_g == NATIVE) {
				return;
			}
			break;
		}
		case LABI_CV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_CV", arg1);
			obj = THIS;
			if (obj->h.t == KEK_CLASS) {
				cls = (class_t*) obj;
			} else {
				cls = obj->h.cls;
			}
			vm_debug(DBG_BC, " - load address of static symbol \"%s\"\n",
					cls->syms_static[arg1].name);
			PUSH(&cls->syms_static[arg1].value);
			break;
		}
		case LVBI_CV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_CV", arg1);
			obj = THIS;
			if (obj->h.t == KEK_CLASS) {
				cls = (class_t*) obj;
			} else {
				cls = obj->h.cls;
			}
			vm_debug(DBG_BC, " - load value of static symbol \"%s\"\n",
					cls->syms_static[arg1].name);
			PUSH(cls->syms_static[arg1].value);
			break;
		}
		case NEW: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "NEW", arg1); // NEW should have a second argument like CALL
			sym = CONST(arg1);
			if (sym->h.t != KEK_SYM) {
				vm_error("Expected symbol as the argument of NEW.\n");
			}
			cls = vm_find_class(sym->k_sym.symbol);
			if (!cls) {
				vm_error("Cannot find class %s.\n", sym->k_sym.symbol);
			}
			obj = cls->allocator(cls); // Allocate tagged memory for the object
			mth = cls->constructor;

			PUSH(obj); // Return value of NEW
			PUSH(obj); // Push instance pointer (THIS)
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
			if (sym->h.t != KEK_SYM) {
				vm_error("Expected symbol as the argument of LD_CLASS.\n");
			}
			cls = vm_find_class(sym->k_sym.symbol);
			PUSH(cls);
			break;
		}
		case LABI_IV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LABI_IV", arg1);
			obj = THIS;
			if (obj->h.t != KEK_UDO) {
				vm_error("Invalid THIS pointer.\n");
			}
			PUSH(&obj->k_udo.inst_var[arg1]);
			break;
		}
		case LVBI_IV: {
			arg1 = BC_OP16(++ip_g);
			ip_g += 2;
			vm_debug(DBG_BC, "%s %u\n", "LVBI_IV", arg1);
			obj = THIS;
			if (obj->h.t != KEK_UDO) {
				vm_error("Invalid THIS pointer.\n");
			}
			PUSH(obj->k_udo.inst_var[arg1]);
			break;
		}
		case LVBI_CVE:
		case LVBS_IVE:
		case LVBS_CVE:
		case LABI_CVE:
		case LABS_IVE:
		case LABS_CVE:

		default:
			vm_error("Invalid instruction at %u\n", ip_g);
			break;
		}
	}
}
