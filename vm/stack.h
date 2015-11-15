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

extern kek_obj_t **stack_g;
extern int stack_size_g;
extern int sp_g; /* stack pointer */
extern int ap_g; /* argument pointer */
extern int fp_g; /* frame pointer */
extern int ip_g; /* instruction pointer */

void stack_init(void);
void stack_destroy(void);
void stack_push(void *);
kek_obj_t* stack_pop();
kek_obj_t* stack_top();

#define ARG(i) stack_g[ap_g + (i)]
#define LOC(i) stack_g[fp_g + (i)]
#define THIS stack_g[fp_g - 4]

#define NATIVE -1

#if defined(__LP64__)
#define PUSH(obj) stack_push((void*)(uint64_t)(obj))
#else
#define PUSH(obj) stack_push((void*)(obj))
#endif

#define POP(obj) (obj) = (void*)stack_pop()
#define TOP(obj) (obj) = (void*)stack_top()

#define BC_CALL(entry, ret, arg_cnt, locals_cnt) { \
	PUSH(ret); \
	PUSH(ap_g); \
	ap_g = sp_g - (arg_cnt) - 3; \
	PUSH(fp_g); \
	fp_g = sp_g; \
	sp_g = sp_g + (locals_cnt); \
	ip_g = entry; \
}

#define BC_RET do { \
	kek_obj_t* ret_val = stack_pop(); \
	sp_g = ap_g; \
	uint32_t ret_addr = (size_t)stack_g[fp_g - 3]; \
	ap_g = (size_t)stack_g[fp_g - 2]; \
	fp_g = (size_t)stack_g[fp_g - 1]; \
	PUSH(ret_val); \
	ip_g = ret_addr; \
} while (0)

#define BC_RETVOID { \
	sp_g = ap_g; \
	uint32_t ret_addr = (size_t)stack_g[fp_g - 3]; \
	ap_g = (size_t)stack_g[fp_g - 2]; \
	fp_g = (size_t)stack_g[fp_g - 1]; \
	ip_g = ret_addr; \
}

// Read 8-bit instruction operand
#define BC_OP8(i) bc_arr_g[(i)]
// Read 16-bit instruction operand
#define BC_OP16(i) *(uint16_t*) &bc_arr_g[(i)]


#endif /* STACK_H_ */
