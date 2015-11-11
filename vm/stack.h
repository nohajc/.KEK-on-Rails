/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include "types.h"

/******************************************************************************/
/* stack **********************************************************************/

kek_obj_t **stack_g;
int stack_size_g;
int sp_g; /* stack pointer */
int fp_g; /* function pointer */

void stack_init(void);
void stack_destroy(void);
void stack_push(kek_obj_t *);
kek_obj_t* stack_pop();
kek_obj_t* stack_top();

#endif /* STACK_H_ */
