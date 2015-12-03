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
header_t *stack_obj;

void stack_init(void) {
	vm_debug(DBG_STACK, "init, size=%d\n", STACK_DEFAULT_SIZE);

	stack_size_g = STACK_DEFAULT_SIZE;
	sp_g = 0;
	fp_g = 0;
	ap_g = 0;

	vm_debug(DBG_STACK, "sizeof(header_t) = %u\n", sizeof(header_t));

#if FORCE_CALLOC == 1
	stack_obj = calloc((sizeof(header_t) + stack_size_g) * sizeof(kek_obj_t*), 1);
#else /* FORCE_CALLOC == 1 */
	stack_obj = malloc((sizeof(header_t) + stack_size_g) * sizeof(kek_obj_t*));
#endif /* FORCE_CALLOC == 1 */
	stack_obj->t = KEK_STACK;
	stack_obj->cls = NULL;
	stack_g = (kek_obj_t **) (stack_obj + 1); // actually stack_obj + 1 * sizeof(header_t)

	assert(stack_g);
}

void stack_destroy(void) {
	vm_debug(DBG_STACK, "destroy\n");
	free(STACK_HEADER(stack_g));
}

void stack_push(void *obj) {
	if (sp_g >= stack_size_g) {
		vm_error("Stack overflowed. Please increase STACK_DEFAULT_SIZE in "
				"stack.h\n");
		exit(1);
	}

	vm_debug(DBG_STACK, "push to   stack[%3d] = %p\n", sp_g, obj);
	stack_g[sp_g++] = obj;
}

kek_obj_t* stack_pop() {
	vm_debug(DBG_STACK, "pop  from stack[%3d] = %p\n", sp_g, stack_g[sp_g - 1]);
	return (stack_g[--sp_g]);
}
kek_obj_t* stack_top() {
	vm_debug(DBG_STACK, "top from stack[%d] = %p\n", sp_g, stack_g[sp_g - 1]);
	return (stack_g[sp_g - 1]);
}
