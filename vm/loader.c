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
/* global variables */

class_t *classes_g = NULL;

/******************************************************************************/

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
		return (NULL);
	}

	fread_result = fread(string, len, sizeof(char), f);
	if (fread_result != (len * sizeof(char))) {
		vm_error("Reading of \"string\" of len=%d has failed."
				"fread_result=%zu\n", len, fread_result);
		return (FALSE);
	}

	return (string);
}

/******************************************************************************/

int kexe_load_sym(FILE *f, symbol_t *sym) {
	/* size_t fread_result; */
	sym->name =  kexe_load_string(f);

	/*sym->const_flag
	kexe_load_uint32
	DruhId type;
	Scope scope;
	uint32_t const_ptr;

		classout_w8(cow, pt->druh);
		classout_w8(cow, pt->sc);
		classout_w32(cow, pt->const_ptr); */
	return (TRUE);
}

/*
 * sym_cnt is a pointer to classes_g->syms_*_cnt
 */
int kexe_load_syms(FILE *f, uint32_t *sym_cnt, symbol_t *sym_array) {
	size_t fread_result;
	uint32_t i;

	/* TODO: load both syms_{static,instance} */

	fread_result = fread(sym_cnt, 1, sizeof(uint32_t), f);
	if (fread_result != sizeof(uint32_t)) {
		vm_error("Reading of \"cym_cnt\" has failed. fread_result=%zu\n",
				fread_result);
		return (FALSE);
	}

	for (i = 0; i < *sym_cnt; i++) {
		if (!kexe_load_sym(f, &sym_array[i])) {
			return (FALSE);
		}
	}

	return (TRUE);
}

int kexe_load_classes(FILE *f) {
	size_t fread_result;
	uint32_t classes_cnt;
	uint32_t i;

	fread_result = fread(&classes_cnt, 1, sizeof(uint32_t), f);
	if (fread_result != sizeof(uint32_t)) {
		vm_error("Reading of \"classes\" has failed. fread_result=%zu\n",
				fread_result);
		return (FALSE);
	}

	vm_debug("classes_cnt=%d\n", classes_cnt);
	classes_g = malloc(classes_cnt * sizeof(class_t));
	assert(classes_g);

	for (i = 0; i < classes_cnt; i++) {
		classes_g[i].name = kexe_load_string(f);
		classes_g[i].parent_name = kexe_load_string(f);
		kexe_load_syms(f, &classes_g[i].syms_static_cnt,
				classes_g[i].syms_static);
		kexe_load_syms(f, &classes_g[i].syms_instance_cnt,
				classes_g[i].syms_instance);
	}

	return (TRUE);
}

int kexe_load(const char *filename) {
	FILE *f;
	uint32_t kek_magic;
	size_t fread_result;

	f = fopen(filename, "r");
	if (f == NULL) {
		vm_error("Filename \"%s\" does not exists.\n");
		return (FALSE);
	}

	/* try to read magic number */
	fread_result = fread(&kek_magic, 1, sizeof(uint32_t), f);
	if (fread_result != sizeof(uint32_t)) {
		vm_error("Reading of the first four bytes has failed.\n");
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
	fclose(f);
	return (FALSE);
}
