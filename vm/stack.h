/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdlib.h>
#include "types.h"
#include "vm.h"
#include "k_integer.h"

/******************************************************************************/
/* stack **********************************************************************/

#define STACK_DEFAULT_SIZE 1024*4

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

#define STACK_HEADER(st) (((header_t*)(st)) - 1)

#define ARG(i) stack_g[ap_g + (i)]
#define LOC(i) stack_g[fp_g + (i) + 1]
#define THIS stack_g[fp_g - 4]

#define NATIVE -1

#if defined(__LP64__)
#define PUSH(obj) stack_push((void*)(uint64_t)(obj))
#else /* defined(__LP64__) */
#define PUSH(obj) stack_push((void*)(obj))
#endif /* defined(__LP64__) */

#define POP(obj) (obj) = (void*)stack_pop()
#define TOP(obj) (obj) = (void*)stack_top()

#define BC_CALL(entry, ret, arg_cnt, locals_cnt) { \
	int i; \
	vm_debug(DBG_STACK, "bc_call: ret\n"); \
	PUSH(make_integer(ret)); \
	vm_debug(DBG_STACK, "bc_call: ap\n"); \
	PUSH(make_integer(ap_g)); \
	ap_g = sp_g - (arg_cnt) - 3; \
	vm_debug(DBG_STACK, "bc_call: fp\n"); \
	PUSH(make_integer(fp_g)); \
	fp_g = sp_g; \
	vm_debug(DBG_STACK, "bc_call: stack[%d (fp_g)] is NULL\n", fp_g); \
	stack_g[fp_g] = NULL; \
	sp_g = sp_g + (locals_cnt) + 1; \
	for (i = fp_g + 1; i < sp_g; ++i) { \
		stack_g[i] = NULL; \
	} \
	ip_g = entry; \
}

// Tail call - we reuse the current stack frame thus destroying it.
/* How it works:
 1. save return address, caller AP and caller FP from the current frame
 2. set tmp AP to SP-arg_cnt-1
 3. copy arguments and instance/class pointer from tmp AP to current AP
 4. set current SP to current AP+arg_cnt+1 (after instance/class pointer)
 5. push return address, caller AP and caller FP after that
 6. set current FP to SP
 7. increment SP by locals_cnt
 8. set IP to function entry point
 */
#define BC_TCALL(entry, arg_cnt, locals_cnt) { \
	int j; \
	uint32_t ret_addr = (size_t)INT_VAL(stack_g[fp_g - 3]); \
	int caller_ap = (size_t)INT_VAL(stack_g[fp_g - 2]); \
	int caller_fp = (size_t)INT_VAL(stack_g[fp_g - 1]); \
	int tmp_ap = sp_g - (arg_cnt) - 1; \
	uint32_t i; \
	for (i = 0; i <= (arg_cnt); ++i) { \
		stack_g[ap_g + i] = stack_g[tmp_ap + i]; \
	} \
	sp_g = ap_g + (arg_cnt) + 1; \
	PUSH(make_integer(ret_addr)); \
	PUSH(make_integer(caller_ap)); \
	PUSH(make_integer(caller_fp)); \
	fp_g = sp_g; \
	stack_g[fp_g] = NULL; \
	sp_g = sp_g + (locals_cnt) + 1; \
	for (j = fp_g + 1; j < sp_g; ++j) { \
		stack_g[j] = NULL; \
	} \
	ip_g = entry; \
}

#define BC_RET do { \
	kek_obj_t* ret_val = stack_pop(); \
	sp_g = ap_g; \
	uint32_t ret_addr = (size_t)INT_VAL(stack_g[fp_g - 3]); \
	ap_g = (size_t)INT_VAL(stack_g[fp_g - 2]); \
	fp_g = (size_t)INT_VAL(stack_g[fp_g - 1]); \
	PUSH(ret_val); \
	ip_g = ret_addr; \
} while (0)

#define BC_RET_SELF { \
	kek_obj_t* ret_val = THIS; \
	sp_g = ap_g; \
	uint32_t ret_addr = (size_t)INT_VAL(stack_g[fp_g - 3]); \
	ap_g = (size_t)INT_VAL(stack_g[fp_g - 2]); \
	fp_g = (size_t)INT_VAL(stack_g[fp_g - 1]); \
	PUSH(ret_val); \
	ip_g = ret_addr; \
}

// Read 8-bit instruction operand
#define BC_OP8(i) bc_arr_g[(i)]
// Read 16-bit instruction operand
#define BC_OP16(i) *(uint16_t*) &bc_arr_g[(i)]

#endif /* STACK_H_ */
