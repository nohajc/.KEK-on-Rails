/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "k_string.h"
#include "vm.h"
#include "memory.h"

void init_kek_string_class(void) {
	char name[] = "String";
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 0;
	// TODO: add native methods such as length(), get_idx(), get_idxa()
	classes_g[classes_cnt_g].methods = NULL;

	classes_g[classes_cnt_g].allocator = NULL;
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].syms_instance_cnt = 0;
	classes_g[classes_cnt_g].syms_instance = NULL;

	classes_g[classes_cnt_g].parent_name = NULL;

	classes_cnt_g++;
}

kek_obj_t * new_string_from_cstring(const char * cstr) {
	size_t len = strlen(cstr);
	class_t * str_class = vm_find_class("String");
	kek_string_t * kstr = (kek_string_t *) alloc_string(str_class, len);
	kstr->length = len;
	strcpy(kstr->string, cstr);

	return ((kek_obj_t *) kstr);
}
