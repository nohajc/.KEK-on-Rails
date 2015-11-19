/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "k_integer.h"
#include "vm.h"
#include "memory.h"

void new_integer(void) {
	kek_int_t * n = (kek_int_t*)THIS;
	int value = (int64_t)ARG(0);

	native_new_integer(n, value);

	BC_RETVOID;
}

void native_new_integer(kek_int_t * n, int value) {
	n->value = value;
}

kek_int_t * make_integer(int value) {
	kek_int_t * ret;
	if (value & 0x80000000) {
		ret = (kek_int_t*)alloc_integer();
		native_new_integer(ret, value);
	}
	else { // Optimization for smaller numbers
		#if defined(__LP64__)
		ret = (kek_int_t*)(((int64_t)value << 1) | 1);
		#else
		ret = (kek_int_t*)(((value << 1) | 1);
		#endif
	}
	return ret;
}