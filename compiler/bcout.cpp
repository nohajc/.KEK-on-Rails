#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "bcout.h"

bcout_t *bcout_g;

/******************************************************************************/
#if DEBUG

void bco_debug(const char *format, va_list ap) {
	va_list args;
	fprintf(stderr, "bco: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	fflush(stderr);
}

#else

void bco_debug(const char *format, va_list ap) {
}

#endif
/******************************************************************************/

bcout_t *bcout_init() {
	bcout_t *bco;

	bco = (bcout_t *) malloc(sizeof(bcout_t));
	assert(bco);

	/* alocate array for bytecode */
	bco->bc_arr_size = 4096;
	bco->bc_arr_cnt = 0;
	bco->bc_arr = (uint8_t *) malloc(bco->bc_arr_size * sizeof(uint8_t));
	assert(bco->bc_arr);

	/* alocate array for bytecode */
	bco->const_table_size = 4096;
	bco->const_table_cnt = 0;
	bco->const_table = (uint8_t *) malloc(
			bco->const_table_size * sizeof(uint8_t *));
	assert(bco->bc_arr);

	/* array of pointers to const_table */
	bco->items_size = 4096;
	bco->items_cnt = 0;
	bco->items = (constant_item_t **) malloc(
			bco->items_size * sizeof(constant_item_t *));
	assert(bco->items);

	return (bco);
}

void bcout_free(bcout_t *bco) {
	free(bco->items);
	free(bco);
}

void bcout_items_resize(bcout_t *bco) {
	bco->items_size *= 2;
	bco->items = (constant_item_t **) realloc(bco->items,
			bco->items_size * sizeof(constant_item_t *));
	assert(bco->items);
}

void bcout_items_add(bcout_t *bco, constant_item_t *i) {
	if (bco->items_cnt >= bco->items_size) {
		bcout_items_resize(bco);
	}
	bco->items[bco->items_cnt++] = i;
}

/******************************************************************************/

void *ct_malloc(bcout_t *bco, size_t obj_size) {
	void *ptr; /* pointer to the beginning of the allocated data */

	ptr = bco->const_table + bco->bc_arr_cnt;

	if (bco->const_table_cnt + obj_size >= bco->const_table_size) {
		bco->const_table_size *= 2;
		bco->const_table = realloc(bco->const_table, bco->const_table_size);
		assert(bco->const_table);
	}

	bco->bc_arr_cnt += obj_size;

	return (ptr);
}

/******************************************************************************/

/* create an integer, save it into the constant table and create
 * a pointer in the items array
 * usage:
 *
 * int i;
 * i = bco_int(bco, 666);
 * constant_int_t *j = (constant_int_t *)(bco->const_table + i));
 *
 */
int bco_int(bcout_t *bco, int v) {
	constant_int_t *ci;
	int found;

	found = bco_find_int(bco, v);
	if (found) {
		return (found);
	}

	ci = (constant_int_t *) ct_malloc(bco, sizeof(constant_int_t));
	ci->type = INT;
	ci->value = v;

	bcout_items_add(bco, (constant_item_t *) ci);

	return ((uint8_t *) ci - bco->const_table);
}

int bco_str(bcout_t *bco, const char *str) {
	constant_string_t *cs;
	size_t len;
	int found;

	found = bco_find_str(bco, str);
	if (found) {
		return (found);
	}

	len = strlen(str);

	cs = (constant_string_t *) ct_malloc(bco, sizeof(constant_string_t) + len);
	cs->type = STRING;
	cs->length = len;
	strcpy(cs->string, str);

	bcout_items_add(bco, (constant_item_t *) cs);

	return ((uint8_t *) cs - bco->const_table);
}

/* tady to vraci spatne */
int bco_find_int(bcout_t *bco, int v) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i].type == INT && bco->items[i].ci.value == v) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (0);
}

/* TODO: check for length first? */
int bco_find_str(bcout_t *bco, const char *str) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i].type == STRING
				&& strcmp(bco->items[i].cs.string, str) == 0) {
			return (i);
		}
	}

	return (0);
}

