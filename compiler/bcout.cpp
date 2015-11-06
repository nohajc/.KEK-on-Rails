#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "bcout.h"

bcout_t *bcout_g;

/******************************************************************************/
#if DEBUG

void bco_debug(const char *format, ...) {
	va_list args;
	fprintf(stderr, "bco: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	fflush(stderr);
}

#else

void bco_debug(const char *format, ...) {
}

#endif
/******************************************************************************/

const char *op_str[] = {
	"UNDEF",
	"BOP",
	"UNM",
	"LD",
	"ST",
	"IFNJ",
	"JU",
	"WRT",
	"RD",
	"DUP",
	"SWAP",
	"NOT",
	"STOP",
	"RET",
	"CALL",
	"CALLS",
	"CALLE",
	"PUSH_C",
	"PUSH_ARG",
	"PUSH_LOC",
	"PUSH_IV",
	"PUSH_CV",
	"PUSH_IVE",
	"PUSH_CVE",
	"PUSH_SELF",
	"CLASSREF",
	"PUSHA_ARG",
	"PUSHA_LOC",
	"PUSHA_IV",
	"PUSHA_CV",
	"PUSHA_IVE",
	"PUSHA_CVE",
	"IDX",
	"IDXA",
	"NEW"
};

bcout_t *bcout_init() {
	bcout_t *bco;

	bco = (bcout_t *) malloc(sizeof(bcout_t));
	assert(bco);

	/* alocate array for bytecode */
	bco->bc_arr_size = 4096;
	bco->bc_arr_cnt = 0;
	bco->bc_arr = (uint8_t *) malloc(bco->bc_arr_size * sizeof(uint8_t));
	assert(bco->bc_arr);
	bco->bc_lab = (uint8_t *) malloc(bco->bc_arr_size * sizeof(uint8_t));
	assert(bco->bc_lab);

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

/******************************************************************************/
/* bytecode utils */

/* check, if we can add 'size' to the array. if not, realloc */
void bc_arr_realloc(bcout_t *bco, size_t size) {
	if (bco->bc_arr_size < bco->bc_arr_cnt + size) {
		bco->bc_arr_size *= 2;
		bco->bc_arr = (uint8_t *) realloc(bco->bc_arr,
				bco->bc_arr_size * sizeof(uint8_t *));
		assert(bco->bc_arr);
		bco->bc_lab = (uint8_t *) realloc(bco->bc_lab,
				bco->bc_arr_size * sizeof(uint8_t *));
		assert(bco->bc_lab);
	}
}

/* write an uint8 to the bc_arr */
uint32_t bco_w8(bcout_t *bco, uint8_t uint8) {
	bc_arr_realloc(bco, sizeof(uint8_t));
	bco->bc_lab[bco->bc_arr_cnt] = NIL;
	bco->bc_arr[bco->bc_arr_cnt++] = uint8;

	return (bco->bc_arr_cnt - 1);
}

uint32_t bco_w8_labeled(bcout_t *bco, uint8_t uint8, label_t lab) {
	bc_arr_realloc(bco, sizeof(uint8_t));
	bco->bc_lab[bco->bc_arr_cnt] = lab;
	bco->bc_arr[bco->bc_arr_cnt++] = uint8;

	return (bco->bc_arr_cnt - 1);
}

/* write an uint16 (as two uint8) to the bc_arr */
uint32_t bco_w16(bcout_t *bco, uint16_t uint16) {
	bc_arr_realloc(bco, sizeof(uint16_t));

	*(uint16_t*) &bco->bc_lab[bco->bc_arr_cnt] = NIL;
	*(uint16_t*) &bco->bc_arr[bco->bc_arr_cnt] = uint16;
	bco->bc_arr_cnt += 2;
	return (bco->bc_arr_cnt - 2);
}

/* write an uint32 (as four uint8) to the bc_arr */
uint32_t bco_w32(bcout_t *bco, uint32_t uint32) {
	bc_arr_realloc(bco, sizeof(uint32_t));

	*(uint32_t*) &bco->bc_lab[bco->bc_arr_cnt] = NIL;
	*(uint32_t*) &bco->bc_arr[bco->bc_arr_cnt] = uint32;
	bco->bc_arr_cnt += 4;
	return (bco->bc_arr_cnt - 4);
}

/* FIXME: the name of this method is probably not good */
uint32_t bco_w0(bcout_t *bco, bc_t bc) {
	uint32_t ip = bco_w8(bco, bc);

	bco_debug("%3d: %s", ip, op_str[bc]);
	return (ip);
}

uint32_t bco_wb1(bcout_t *bco, bc_t bc, uint8_t arg) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w8(bco, arg);

	bco_debug("%3d: %s %u", ip, op_str[bc], arg);
	return (ip);
}

uint32_t bco_ww1(bcout_t *bco, bc_t bc, uint16_t arg) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w16(bco, arg);

	bco_debug("%3d: %s %u", ip, op_str[bc], arg);
	return (ip);
}

uint32_t bco_ww1_labeled(bcout_t *bco, bc_t bc, uint16_t arg, label_t lab){
	uint32_t ip;

	ip = bco_w8_labeled(bco, bc, lab);
	(void) bco_w16(bco, arg);

	bco_debug("%3d: %s %u", ip, op_str[bc], arg);
	return (ip);
}

uint32_t bco_ww2(bcout_t *bco, bc_t bc, uint16_t arg0, uint16_t arg1) {
	uint32_t ip;

	ip = bco_w8(bco, bc);
	(void) bco_w16(bco, arg0);
	(void) bco_w16(bco, arg1);

	bco_debug("%3d: %s %u %u", ip, op_str[bc], arg0, arg1);
	return (ip);
}

/******************************************************************************/
/* constat table utils */

void bcout_items_add(bcout_t *bco, constant_item_t *i) {
	if (bco->items_size <= bco->items_cnt) {
		bco->items_size *= 2;
		bco->items = (constant_item_t **) realloc(bco->items,
				bco->items_size * sizeof(constant_item_t *));
		assert(bco->items);
	}
	bco->items[bco->items_cnt++] = i;
}

void *ct_malloc(bcout_t *bco, size_t obj_size) {
	void *ptr; /* pointer to the beginning of the allocated data */

	ptr = bco->const_table + bco->const_table_cnt;

	if (bco->const_table_size <= bco->const_table_cnt + obj_size) {
		bco->const_table_size *= 2;
		bco->const_table = (uint8_t*) realloc(bco->const_table,
				bco->const_table_size);
		assert(bco->const_table);
	}

	bco->const_table_cnt += obj_size;

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
uint32_t bco_int(bcout_t *bco, int v) {
	constant_int_t *ci;
	int found;

	found = bco_find_int(bco, v);
	if (-1 < found) {
		return (found);
	}

	ci = (constant_int_t *) ct_malloc(bco, sizeof(constant_int_t));
	ci->type = INT;
	ci->value = v;

	bcout_items_add(bco, (constant_item_t *) ci);

	return ((uint8_t *) ci - bco->const_table);
}

uint32_t bco_str(bcout_t *bco, const char *str) {
	constant_string_t *cs;
	size_t len;
	int found;

	found = bco_find_str(bco, str);
	if (-1 < found) {
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

uint32_t bco_sym(bcout_t *bco, const char *str) {
	constant_string_t *cs;
	size_t len;
	int found;

	found = bco_find_sym(bco, str);
	if (-1 < found) {
		return (found);
	}

	len = strlen(str);

	cs = (constant_string_t *) ct_malloc(bco, sizeof(constant_string_t) + len);
	cs->type = SYMBOL;
	cs->length = len;
	strcpy(cs->string, str);

	bcout_items_add(bco, (constant_item_t *) cs);

	return ((uint8_t *) cs - bco->const_table);
}

size_t bco_get_ip(bcout_t *bco) {
	return bco->bc_arr_cnt;
}

void bco_fix_forward_jmpw(bcout_t *bco, size_t idx) {
	*(uint16_t*) &bco->bc_arr[idx + 1] = (uint16_t) idx;
}

void bco_resolve_break(bcout_t *bco, size_t a1, size_t a2) {
	for (size_t i = a1; i < a2; ++i) {
		if (bco->bc_lab[i] == BRK) {
			bco_fix_forward_jmpw(bco, i);
			bco->bc_lab[i] = NIL;
		}
	}
}

uint32_t bco_find_int(bcout_t *bco, int v) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == INT && bco->items[i]->ci.value == v) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}

uint32_t bco_find_str(bcout_t *bco, const char *str) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == STRING
				&& strcmp(bco->items[i]->cs.string, str) == 0) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}

uint32_t bco_find_sym(bcout_t *bco, const char *str) {
	int i;

	for (i = 0; i < bco->items_cnt; i++) {
		if (bco->items[i]->type == SYMBOL
				&& strcmp(bco->items[i]->cs.string, str) == 0) {
			return ((uint8_t *) bco->items[i] - bco->const_table);
		}
	}

	return (-1);
}
