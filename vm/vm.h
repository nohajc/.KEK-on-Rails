/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include "bc.h"
#include "types.h"

/* stolen from "../compiler/tabsym.h" */
/* We don't need DruhId and Scope in the final structures
 * LOCAL and ARG variable's names won't be stored
 *     - they're always referenced by address on the stack
 * INSTANCE and CLASS type is determined by the location of the symbol:
 * syms_instance or syms_static in the class_t structure.
 * Instance symbols are always variables (IdProm).
 * Static class symbols will have the const_flag_t flag.
 * We also don't need IdConstNum and IdConstStr
 *     - the type is determined by object's header.
 */
enum DruhId {
	Nedef, IdProm, IdConstNum, IdConstStr
};
enum Scope {
	SC_LOCAL, SC_ARG, SC_INSTANCE, // instance variable
	SC_CLASS // class static variable
};

/******************************************************************************/

#define DEBUG 1
#define TRUE 1
#define FALSE 0
#define KEK_MAGIC 0x42666CEC

/******************************************************************************/

void vm_debug(const char *format, ...);
void vm_error(const char *format, ...);

/******************************************************************************/
/*
 github.com/nohajc/.KEK-on-Rails/wiki/Class-hierarchy-representation-in-the-VM
 */

/* Native methods also work with our stack
 * so no arguments are passed via the system stack.
 */

struct _class;

typedef void (*method_ptr)(void);
typedef union _kek_obj *(*alloc_ptr)(struct _class *);

typedef enum _const_flag {
	VAR = 0, CONST = 1
} const_flag_t;

typedef struct _symbol {
	char *name;
	/* There is an index to const table in the class file. At runtime though,
	 * there should be a kek_obj_t pointer for every static var/const instead.
	 * So, there should be two versions of this structure (and possibly others)
	 * one in loader.h reflecting the format of stored binary data
	 * and one here with the runtime format.
	 *
	 * The pointer is used for static symbols only. Pointers to instance vars
	 * are stored in each object's kek_udo_t structure. symbol_t is used here
	 * only for name-based dynamic lookup. You then use addr index to access
	 * the right element in kek_udo_t.
	 */
	uint32_t const_ptr;
	uint32_t addr;
	const_flag_t const_flag;
} symbol_t;

typedef struct _method {
	char *name;
	union _entry {
		uint32_t bc_addr;
		method_ptr func;
	} entry;
	uint32_t args_cnt;
	/* We need this number to properly set SP
	 after call, thus reserving space for locals. */
	uint32_t locals_cnt;
	uint8_t is_static;
	uint8_t is_native;
} method_t;

typedef struct _class {
	char *name;
	struct _class *parent;

	uint32_t methods_cnt;
	method_t *methods;

	alloc_ptr allocator;

	method_t *constructor;
	method_t *static_init; /* "Static constructor" */

	uint32_t syms_static_cnt;
	symbol_t *syms_static;

	uint32_t syms_instance_cnt;
	symbol_t *syms_instance;

	/* helpers */
	char *parent_name;
} class_t;



#define BUILTIN_CLASSES_CNT 16 // Maybe less but we reserve it for the future

/******************************************************************************/
/* global variables */

extern uint32_t classes_cnt_g;
extern class_t *classes_g;

extern size_t const_table_cnt_g;
extern uint8_t *const_table_g;

extern size_t bc_arr_cnt_g;
extern uint8_t *bc_arr_g;

/******************************************************************************/

#define CONST(i) ((kek_obj_t*)&const_table_g[i])

#define P32 "%u" /* printf uint32_t */
#define P16 "%u" /* printf uint32_t */
#define P8 "%u" /* printf uint32_t */
#define PSIZE "%zu" /* printf size_t */

/******************************************************************************/

void vm_init_builtin_classes(void);
void vm_init_native_method(method_t * mth, const char * name, uint32_t args_cnt,
		uint8_t is_static, method_ptr func);
class_t * vm_find_class(const char * name);

#endif /* VM_H_ */
