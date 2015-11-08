/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef VM_H_
#define VM_H_

#include <stdint.h>

#include "stack.h"

/* stolen from "../compiler/tabsym.h" */
enum DruhId {
	Nedef, IdProm, IdConstNum, IdConstStr
};
enum Scope {
	SC_LOCAL,
	SC_ARG,
	SC_INSTANCE, // instance variable
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

typedef enum _const_flag {
	VAR = 0,
	CONST = 1
} const_flag_t;

typedef struct _symbol {
	const char *name;
	uint32_t const_ptr;
	uint32_t addr;
	const_flag_t const_flag;
} symbol_t;

typedef struct _method {
	const char *name;
} method_t;

typedef struct _class {
	const char *name;
	struct _class *parent;

	uint32_t methods_cnt;
	method_t *methods;

	uint32_t syms_static_cnt;
	symbol_t *syms_static;

	uint32_t syms_instance_cnt;
	symbol_t *syms_instance;

	/* helpers */
	const char *parent_name;
} class_t;

/******************************************************************************/
/* global variables */

extern class_t *classes_g;

/******************************************************************************/

#endif /* VM_H_ */
