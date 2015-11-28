/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "vm.h"
#include "stack.h"

kek_obj_t **stack_g;
int stack_size_g;
int sp_g; /* stack pointer */
int ap_g; /* argument pointer */
int fp_g; /* frame pointer */
int ip_g; /* instruction pointer */

void stack_init(void) {
	stack_size_g = STACK_DEFAULT_SIZE;
	sp_g = 0;
	fp_g = 0;

	stack_g = malloc(stack_size_g * sizeof(kek_obj_t*));
	assert(stack_g);
}

void stack_destroy(void) {
	free(stack_g);
}

void stack_push(void *obj) {
	if (sp_g >= stack_size_g) {
		stack_size_g *= 2;
		stack_g = realloc(stack_g, stack_size_g * sizeof(kek_obj_t*));
		assert(stack_g != NULL);
		vm_debug(DBG_STACK,
				"== stack realloced stack=%p size=%lu sizeof(kek_obj_t)=%lu \n",
				stack_g, stack_size_g, sizeof(kek_obj_t));
		assert(0 && "stack reallocing do not work right");
	}

	/* FIXME
	 if (IS_PTR(obj)) {
		vm_debug(DBG_STACK, "stack_push: \"%s\"\n", kek_obj_print(obj));
	}*/

	stack_g[sp_g++] = obj;
}

kek_obj_t* stack_pop() {
	/*vm_debug(DBG_STACK, "stack_pop: \"%s\"\n",
	 kek_obj_print(stack_g[sp_g - 1]));*/
	return (stack_g[--sp_g]);
}
kek_obj_t* stack_top() {
	/*vm_debug(DBG_STACK, "stack_top: \"%s\"\n",
	 kek_obj_print(stack_g[sp_g - 1]));*/
	return (stack_g[sp_g - 1]);
}
