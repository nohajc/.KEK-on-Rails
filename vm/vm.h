/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef VM_H_
#define VM_H_

#include "stack.h"

#define DEBUG 1
#define KEK_MAGIC 0x42666CEC

void vm_debug(const char *format, ...);
void vm_error(const char *format, ...);

typedef enum _arg_type {
	ARG_INTEGER, /* */
	ARG_STRING, /* */
	ARG_BOOLEAN, /* */
	ARG_OBJECT /* */
} arg_type_t;

typedef struct _arg {
	arg_type_t type;
} arg_t;

typedef struct _method {
	const char *name;
} method_t;

typedef struct _class {
	const char *name;
	struct _class *p; /* parent */
	/* methods */
	/* variables */
} class_t;

/* typedef struct _vm {
} vm_t; */

void program_load(const char *filename);
class_t *class_find(const char *key);
method_t *method_find(class_t *class, const char *key);
void method_eval(method_t *method);
class_t *class_new(const char *name);
void class_free(class_t *class);

#endif /* VM_H_ */
