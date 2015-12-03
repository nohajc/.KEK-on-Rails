/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef VM_H_
#define VM_H_

#include <stdint.h>
#include <stdbool.h>
#include "bc.h"
#include "types.h"

#define KEK_MAGIC 0x42666CEC

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
/* error handling */

#define EXIT_ON_ERROR 1 /* if not, kek will try to run even after an error */
#define BRUTAL_KILL 1 /* valgrind tell us stack trace on error */

#define FORCE_CALLOC 0

/******************************************************************************/
/* debugging */

extern uint32_t ticks_g;

#define DEBUG 1
#define DBG_NONE		0x00000000 /* no debug */
#define DBG_LOADING		0x00000001 /* loading */
#define DBG_BC			0x00000002 /* bytecode */
#define DBG_STACK		0x00000004 /* what happens on top of the stack */
#define DBG_STACK_FULL	0x00000008 /* print all stack after every stack manip */
#define DBG_VM			0x00000010 /* virtual machine */
#define DBG_GC			0x00000020 /* garbage collector */
#define DBG_MEM			0x00000080 /* memory */
#define DBG_OBJ_TBL		0x00000100 /* object table */
#define DBG_GC_STATS	0x00000200 /* gc stats */

#define DBG_ALL (DBG_LOADING|DBG_BC|DBG_STACK|DBG_STACK_FULL|DBG_VM|DBG_BC| \
	DBG_OBJ_TBL|DBG_GC_STATS)

void vm_debug(uint32_t level, const char *format, ...);
void vm_error(const char *format, ...);
char *kek_obj_print(kek_obj_t *kek_obj);

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
	kek_obj_t * value;
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
	type_t t;
	char *name;
	struct _class *parent;

	uint32_t methods_cnt;
	method_t *methods;

	alloc_ptr allocator;

	method_t *constructor;
	method_t *static_init; /* "Static constructor" */

	uint32_t syms_static_cnt;
	symbol_t *syms_static;

	int total_syms_instance_cnt; /* Including inherited members */
	uint32_t syms_instance_cnt;
	symbol_t *syms_instance;
	int syms_instance_offset;

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

extern uint32_t debug_level_g;
extern uint32_t test_g;

/******************************************************************************/

#define CONST(i) ((kek_obj_t*)&const_table_g[i])

#define P32 "%u" /* printf uint32_t */
#define P16 "%u" /* printf uint32_t */
#define P8 "%u" /* printf uint32_t */
#define PSIZE "%zu" /* printf size_t */

/******************************************************************************/

void vm_init_builtin_classes(void);
void vm_init_parent_pointers(void);
void vm_init_const_table_elems(void);
void vm_init_native_method(method_t * mth, const char * name, uint32_t args_cnt,
		uint8_t is_static, method_ptr func);
class_t * vm_find_class(const char * name);
method_t * vm_find_method_in_class(class_t * cls, const char * name,
		bool is_static); // searches in a given class
symbol_t * vm_find_static_sym_in_class(class_t * cls, const char * name);
symbol_t * vm_find_instance_sym_in_class(class_t * cls, const char * name);
method_t * vm_find_method(const char * name, bool is_static, class_t ** cls); // returns class where the method was found
void vm_call_class_initializers(void);
void vm_call_main(int argc, char *argv[]);
void vm_execute_bc(void);
void vm_throw_obj(kek_obj_t * obj);
void vm_throw_obj_from_native_ctxt(kek_obj_t * obj);
bool vm_is_const(kek_obj_t *obj);
size_t vm_obj_size(kek_obj_t *obj);
size_t vm_type_size(type_t type);

#endif /* VM_H_ */
