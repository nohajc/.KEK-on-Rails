/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "k_string.h"
#include "k_integer.h"
#include "k_array.h"
#include "vm.h"
#include "memory.h"

void init_kek_string_class(void) {
	char name[] = "String";
	assert(classes_g);
	classes_g[classes_cnt_g].t = KEK_CLASS;
	classes_g[classes_cnt_g].name = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(classes_g[classes_cnt_g].name, name);

	classes_g[classes_cnt_g].parent = NULL;
	classes_g[classes_cnt_g].methods_cnt = 5;

	classes_g[classes_cnt_g].methods = malloc(
		classes_g[classes_cnt_g].methods_cnt * sizeof(method_t));
	vm_init_native_method(&classes_g[classes_cnt_g].methods[0], "length", 0, false, string_length);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[1], "split", 1, false, string_split);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[2], "toInt", 0, false, string_toInt);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[3], "fromArray", 1, true, string_fromArray);
	vm_init_native_method(&classes_g[classes_cnt_g].methods[4], "fromInt", 1, true, string_fromInt);

	classes_g[classes_cnt_g].allocator = NULL;
	classes_g[classes_cnt_g].constructor = NULL;
	classes_g[classes_cnt_g].static_init = NULL;

	classes_g[classes_cnt_g].syms_static_cnt = 0;
	classes_g[classes_cnt_g].syms_static = NULL;
	classes_g[classes_cnt_g].total_syms_instance_cnt = 0;
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

kek_obj_t * new_string_from_concat(const char * a, const char * b) {
	kek_string_t * kstr;
	class_t * str_class = vm_find_class("String");
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);
	size_t c_len = a_len + b_len;

	kstr = (kek_string_t *) alloc_string(str_class, c_len);
	kstr->length = c_len;
	strcpy(kstr->string, a);
	strcpy(kstr->string + a_len, b);

	return ((kek_obj_t *) kstr);
}

void string_length(void) {
	kek_string_t * str = (kek_string_t*)THIS;
	kek_int_t * kek_len = make_integer(str->length);

	PUSH(kek_len);
	BC_RET;
}

void string_split(void) {
	kek_string_t * str = (kek_string_t*)THIS;
	kek_obj_t * delims = ARG(0);
	class_t * arr_class;
	kek_array_t * arr;
	char * token;
	kek_obj_t * tok_str;
	int i = 0;
	uint32_t id = gc_rootset_add((kek_obj_t **) &arr);

	if (!IS_STR(delims)) {
		vm_error("Expected string as argument.\n");
	}

	arr_class = vm_find_class("Array");
	arr = (kek_array_t*) alloc_array(arr_class);
	native_new_array(arr);
	str = (kek_string_t*)THIS; // Update pointer if necessary
	delims = ARG(0);

	// TODO: FIXME, we cannot use strtok safely
	// when str->string pointer can change
	token = strtok(str->string, delims->k_str.string);
	while (token) {
		tok_str = new_string_from_cstring(token);
		native_arr_elem_set(arr, i++, tok_str);
		delims = ARG(0);
		token = strtok(NULL, delims->k_str.string);
	}

	PUSH(arr);
	BC_RET;
	gc_rootset_remove_id(id);
}

void string_toInt(void) {
	kek_string_t * str = (kek_string_t*)THIS;
	kek_int_t * kek_n;
	int n, pos = 0;
	int matched = sscanf(str->string, "%d%n", &n, &pos);
	while (pos < str->length && isspace(str->string[pos])) pos++;

	if (matched != 1 || pos != str->length) {
		PUSH(NIL);
		BC_RET;
		return;
	}
	kek_n = make_integer(n);
	PUSH(kek_n);
	BC_RET;
}

void string_fromInt(void) {
	kek_obj_t * obj = ARG(0);
	kek_obj_t * str;
	char buf[16];

	if (!IS_INT(obj)) {
		vm_error("Expected integer as argument.\n");
	}

	sprintf(buf, "%d", INT_VAL(obj));
	str = new_string_from_cstring(buf);

	PUSH(str);
	BC_RET;
}

void string_fromArray(void) {
	kek_obj_t * obj = ARG(0);
	kek_array_t * arr;
	kek_obj_t * str;
	int i;
	char * buf;

	if (!IS_ARR(obj)) {
		vm_error("Expected array as argument.\n");
	}

	arr = &obj->k_arr;
	buf = malloc((arr->length + 1) * sizeof(char));

	for (i = 0; i < arr->length; ++i) {
		kek_obj_t * el = arr->elems[i];
		if (IS_CHAR(el)) {
			buf[i] = CHAR_VAL(el);
		}
		else {
			buf[i] = ' ';
		}
	}
	buf[i] = '\0';
	str = new_string_from_cstring(buf);
	free(buf);

	PUSH(str);
	BC_RET;
}
