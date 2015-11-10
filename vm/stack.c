/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <assert.h>

#include "stack.h"

void stack_init(void) {
	stack_size_g = 1024;
	sp_g = 0;
	fp_g = 0;

	stack_g = malloc(stack_size_g * sizeof(kek_obj_t));
	assert(stack_g);
}

void stack_destroy(void) {
	free(stack_g);
}

void stack_push(kek_obj_t *obj) {
	if (sp_g == stack_size_g) {
		stack_size_g *= 2;
		stack_g = realloc(stack_g, stack_size_g * sizeof(kek_obj_t));
		assert(stack_g != NULL);
	}
	stack_g[sp_g++] = obj;
}

kek_obj_t* stack_pop() {
	return (stack_g[--sp_g]);
}
kek_obj_t* stack_top() {
	return (stack_g[sp_g - 1]);
}
