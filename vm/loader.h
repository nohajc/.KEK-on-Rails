/*
 * loader.h
 *
 *  Created on: Nov 7, 2015
 *      Author: n
 */

#ifndef VM_LOADER_H_
#define VM_LOADER_H_

#include <stdbool.h>
#include "vm.h"

typedef struct _constant_array {
	header_t h;
	int length;
	int alloc_size;
	// This is a little hack: at runtime, the last two members will be replaced
	//     by pointer to memory allocated elsewhere.
	// Padding is here in case of 64-bit pointers and an array with one element.
	// The elements won't be inlined because the type is mutable but we need to
	//     keep pointer to the array object constant
	// when reallocating the array contents somewhere else (when we grow the
	//     array for example).
	uint32_t padding;
	// This member contains offsets to the array elements (as stored in the
	//     constant table). The array of offsets is inlined.
	uint32_t elems[1];
	// When we load the array object, we allocate array of pointers and then
	//     store the actual element pointers in it.
	// The pointers will be computed from offsets and constant table location
	//     in memory (after it's loaded).
} constant_array_t;

bool kexe_load(const char *filename);
void class_free(class_t *class);

#endif /* VM_LOADER_H_ */
