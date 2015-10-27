/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>

/******************************************************************************/
/* objects ********************************************************************/

typedef enum _tag {
	TAG_INTEGER, /* */
	TAG_STRING, /* */
	TAG_TRUE, /* */
	TAG_FALSE, /* */
	TAG_REFERENCE /* */
} tag_t;

typedef struct _header {
	tag_t t;
} header_t;

typedef struct _obj_integer {
	header_t h;
	int value;
} obj_integer_t;

typedef struct _obj_string {
	header_t h;
	int length;
	char *string;
} obj_string_t;

typedef union _obj {
	obj_integer_t integer;
	obj_string_t string;
} obj_t;

/******************************************************************************/
/* stack **********************************************************************/

typedef enum _bc {
	BC_PUSH = 1, /* */
	BC_PUSH_CLASSREF, /* for calling static method or getting static variables */
	BC_PUSH_ARG, /* */
	BC_POP, /* */
	BC_CALL /* */
} bc_t;

obj_t **stack_g;
int stack_size_g;
int sp_g; /* stack pointer */
int fp_g; /* function pointer */

void stack_init(void);
void stack_destroy(void);
void stack_push(obj_t *);
obj_t* stack_pop();
obj_t* stack_top();

#endif /* STACK_H_ */
