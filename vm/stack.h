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
int ap_g; /* argument pointer */
int fp_g; /* frame pointer */

void stack_init(void);
void stack_destroy(void);
void stack_push(kek_obj_t *);
kek_obj_t* stack_pop();
kek_obj_t* stack_top();

#define ARG(i) stack_g[ap_g + (i)]
#define LOC(i) stack_g[fp_g + (i) + 1]
#define THIS stack_g[fp_g - 3]

#define PUSH(obj) stack_g[sp_g++] = obj
#define POP(obj)  obj = stack_g[--sp_g]
#define TOP(obj)  obj = stack_g[sp_g - 1]

#endif /* STACK_H_ */
