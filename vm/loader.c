/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "vm.h"
#include "loader.h"

/******************************************************************************/
/* global variables. (their extern is in vm.h) */

uint32_t classes_cnt_g = 0;
class_t *classes_g = NULL;

/******************************************************************************/
/* loading of primitives */

uint8_t kexe_load_uint8(FILE *f) {
	size_t fread_result;
	uint32_t ret;

	fread_result = fread(&ret, 1, sizeof(uint8_t), f);
	if (fread_result != sizeof(uint8_t)) {
		vm_error("Reading of \"uint8_t\" has failed. fread_result=%zu\n",
				fread_result);
		return (UINT8_MAX);
	}

	return (ret);
}

uint16_t kexe_load_uint16(FILE *f) {
	size_t fread_result;
	uint32_t ret;

	fread_result = fread(&ret, 1, sizeof(uint16_t), f);
	if (fread_result != sizeof(uint16_t)) {
		vm_error("Reading of \"uint16_t\" has failed. fread_result=%zu\n",
				fread_result);
		return (UINT16_MAX);
	}

	return (ret);
}

uint32_t kexe_load_uint32(FILE *f) {
	size_t fread_result;
	uint32_t ret;

	fread_result = fread(&ret, 1, sizeof(uint32_t), f);
	if (fread_result != sizeof(uint32_t)) {
		vm_error("Reading of \"uint32_t\" has failed. fread_result=%zu\n",
				fread_result);
		return (UINT32_MAX);
	}

	return (ret);
}

char *kexe_load_string(FILE *f) {
	uint32_t len;
	size_t fread_result;
	char *string;

	len = kexe_load_uint32(f);
	if (len == UINT32_MAX) {
		vm_error("Failed to read the \"len\" of the string.\n");
		return (NULL);
	} else {
		vm_debug("Length of the string is " P32 "\n", len);
	}

	string = malloc(len * sizeof(char));
	assert(string);

	/* FIXME: why this works? */
	fread_result = fread(string, sizeof(char), len, f);
	string[len - 1] = '\0';
	if (fread_result != (len * sizeof(char))) {
		vm_error("Reading of a \"string\" of len=%d has failed, "
				"fread_result=%u, should read=%u\n", len, fread_result,
				(len * sizeof(char)));
		return (FALSE);
	} else {
		vm_debug("String \"%s\" has been loaded.\n", string);
	}

	return (string);
}

/******************************************************************************/

int kexe_load_sym(FILE *f, symbol_t *sym) {
	/* size_t fread_result; */
	sym->name = kexe_load_string(f);
	if (sym->name == NULL) {
		vm_error("sym->name failed\n");
		return (FALSE);
	} else {
		vm_debug("loaded symbol = %s", sym->name);
	}

	sym->addr = kexe_load_uint32(f);
	if (sym->addr == UINT32_MAX) {
		vm_error("sym->addr failed\n");
		return (FALSE);
	} else {
		vm_debug("addr of symbol \"%s\" is " P32 "\n", sym->name, sym->addr);
	}

	sym->const_ptr = kexe_load_uint32(f);
	if (sym->addr == UINT32_MAX) {
		vm_error("sym->const_ptr failed\n");
		return (FALSE);
	} else {
		vm_debug("const ptr of symbol \"%s\" is " P32 "\n", sym->name,
				sym->const_ptr);
	}

	sym->const_flag = 0; /* FIXME TODO */

	return (TRUE);
}

/*
 * sym_cnt is a pointer to classes_g->syms_*_cnt
 */
int kexe_load_syms(FILE *f, uint32_t *sym_cnt, symbol_t *sym_array) {
	uint32_t i;

	*sym_cnt = kexe_load_uint32(f);
	if (*sym_cnt == UINT32_MAX) {
		vm_error("Reading of sym_cnt has failed.\n");
		return (FALSE);
	} else {
		vm_debug("*sym_cnt = " P32 "\n", *sym_cnt);
	}

	sym_array = malloc(*sym_cnt * sizeof(symbol_t));
	assert(sym_array);

	/* FIXME: this is in another function. refactor the rest of the loading? */
	for (i = 0; i < *sym_cnt; i++) {
		if (!kexe_load_sym(f, &sym_array[i])) {
			vm_error("loading sym_array[%d] failed\n", i);
			return (FALSE);
		}
	}

	return (TRUE);
}

int kexe_load_methods(FILE *f, uint32_t *methods_cnt, method_t *methods) {
	uint32_t i;

	*methods_cnt = kexe_load_uint32(f);
	if (*methods_cnt == UINT32_MAX) {
		vm_error("Reading of methods_cnt has failed.\n");
		return (FALSE);
	}

	methods = malloc(*methods_cnt * sizeof(method_t));
	assert(methods);

	for (i = 0; i < *methods_cnt; i++) {
		methods[i].name = kexe_load_string(f);
		if (methods[i].name == NULL) {
			vm_error("loading methods[%d].name failed\n", i);
			return (FALSE);
		} else {
			vm_debug("methods[i].name = %s", i, methods[i].name);
		}

		methods[i].args_cnt = kexe_load_uint32(f);
		if (methods[i].args_cnt == UINT32_MAX) {
			vm_error("Reading of methods[%d].args_cnt has failed.\n", i);
			return (FALSE);
		} else {
			vm_debug("methods[i].args_cnt = " P32 "\n", i, methods[i].args_cnt);
		}

		methods[i].locals_cnt = kexe_load_uint32(f);
		if (methods[i].locals_cnt == UINT32_MAX) {
			vm_error("Reading of methods[%d].locals_cnt has failed.\n", i);
			return (FALSE);
		} else {
			vm_debug("methods[i].locals_cnt = " P32 "\n", i,
					methods[i].locals_cnt);
		}

		methods[i].entry.bc_addr = kexe_load_uint32(f);
		if (methods[i].entry.bc_addr == UINT32_MAX) {
			vm_error("Reading of methods[%d].entry.bc_addr has failed.\n", i);
			return (FALSE);
		} else {
			vm_debug("methods[i].entry.bc_addr = " P32 "\n", i,
					methods[i].entry.bc_addr);
		}

		methods[i].is_static = kexe_load_uint8(f);
		if (methods[i].is_static == UINT8_MAX) {
			vm_error("Reading of methods[%d].is_static has failed.\n", i);
			return (FALSE);
		} else {
			vm_debug("methods[i].is_static = " P8 "\n", i,
					methods[i].is_static);
		}
	}

	return (TRUE);
}

int kexe_load_classes(FILE *f) {
	uint32_t i;

	classes_cnt_g = kexe_load_uint32(f);
	if (classes_cnt_g == UINT32_MAX) {
		vm_error("Reading of \"classes\" has failed.\n");
		return (FALSE);
	} else {
		vm_debug("classes_cnt_g = " P32 "\n", classes_cnt_g);
	}

	classes_g = malloc(classes_cnt_g * sizeof(class_t));
	assert(classes_g);

	for (i = 0; i < classes_cnt_g; i++) {
		vm_debug("kexe_load_classes i=%u\n", i);

		classes_g[i].name = kexe_load_string(f);
		if (classes_g[i].name == NULL) {
			vm_error("loading classes_g[%d].name failed\n", i);
			return (FALSE);
		} else {
			vm_debug("classes_g[%d].name = \"%s\"\n", i, classes_g[i].name);
		}

		classes_g[i].parent_name = kexe_load_string(f);
		if (classes_g[i].parent_name == NULL) {
			vm_error("loading classes_g[%d].parent_name failed\n", i);
			return (FALSE);
		} else {
			vm_debug("classes_g[%d].parent_name = %s\n", i,
					classes_g[i].parent_name);
		}

		if (!kexe_load_syms(f, &classes_g[i].syms_static_cnt,
				classes_g[i].syms_static)) {
			vm_error("loading classes_g[%d].syms_static{,_cnt} failed\n", i);
			return (FALSE);
		}

		if (!kexe_load_syms(f, &classes_g[i].syms_instance_cnt,
				classes_g[i].syms_instance)) {
			vm_error("loading classes_g[%d].syms_instance{,_cnt} failed\n", i);
			return (FALSE);
		}

		if (!kexe_load_methods(f, &classes_g[i].methods_cnt,
				classes_g[i].methods)) {
			vm_error("loading classes_g[%d].methods{,_cnt} failed\n", i);
			return (FALSE);
		}
	}

	return (TRUE);
}

int kexe_load(const char *filename) {
	FILE *f;
	uint32_t kek_magic;

	f = fopen(filename, "r");
	if (f == NULL) {
		vm_error("Filename \"%s\" does not exists.\n", filename);
		return (FALSE);
	}

	kek_magic = kexe_load_uint32(f);
	if (kek_magic == UINT32_MAX) {
		vm_error("Failed to load first 32 bits for magic number.\n");
		goto error;
	}

	if (kek_magic != KEK_MAGIC) {
		vm_error("Loaded magic number is not 0x%08x.\n", KEK_MAGIC);
		goto error;
	}

	if (!kexe_load_classes(f)) {
		vm_error("kexe_load_classes has failed.\n");
		goto error;
	}

	fclose(f);
	return (TRUE);

	error: /* cleanup */
	/* TODO: cleanup */
	fclose(f);
	return (FALSE);
}
