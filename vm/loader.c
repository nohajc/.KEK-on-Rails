/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"
#include "loader.h"

/******************************************************************************/
/* global variables */

class_t *classes_g;

/******************************************************************************/

int kexe_load_classes(FILE *f) {
	size_t fread_result;
	uint32_t classes_cnt;

	fread_result = fread(&classes_cnt, 1, sizeof(uint32_t), f);
	if (fread_result != sizeof(uint32_t)) {
		vm_error("Reading of \"classes\" has failed. fread_result=%zu\n",
				fread_result);
		return (FALSE);
	}

	vm_debug("classes_cnt=%d\n", classes_cnt);

	return (TRUE);
}

kexe_t *kexe_load(const char *filename) {
	kexe_t *kexe;
	FILE *f;
	uint32_t kek_magic;
	size_t fread_result;

	f = fopen(filename, "r");
	if (f == NULL) {
		vm_error("Filename \"%s\" does not exists.\n");
		return (NULL);
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
	return (kexe);

	error: /* cleanup */
	fclose(f);
	return (NULL);
}
