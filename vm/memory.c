/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include "memory.h"

kek_obj_t * alloc_array(void) {
	return malloc(sizeof(kek_array_t));
}

kek_obj_t ** alloc_arr_elems(size_t size){
	return malloc(size * sizeof(kek_obj_t*));
}
