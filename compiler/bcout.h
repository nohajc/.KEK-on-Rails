/*
 * bcout.h
 *
 *  Created on: Nov 3, 2015
 *      Author: n
 */

#include <stdint.h>

#ifndef COMPILER_BCOUT_H_
#define COMPILER_BCOUT_H_


typedef enum _constant_type {
	INT, STRING, SYMBOL, ARR
} constant_type_t;

typedef struct _constant_int {
	constant_type_t type;
	int value;
} constant_int_t;

typedef struct _constant_string {
	constant_type_t type;
	int length;
	char string[1];
} constant_string_t;

typedef union _constant_item {
	constant_type_t type;
	constant_int_t ci;
	constant_string_t cs;
} constant_item_t;

typedef struct _bcout {

	int bc_arr_cnt;
	int bc_arr_size;
	uint8_t *bc_arr; /* bytecode array */

	int const_table_cnt;
	int const_table_size;
	uint8_t *const_table;

	int items_cnt;
	int items_size;
	constant_item_t **items;

} bcout_t;

/* todo, drzet si co uz tam mam, abych mohl rict, ze uz to tam je */
/* todo, interface pridej do tabulky constant a vrat offset a pridej instrukci */

bcout_t *bcout_init();
void bcout_free(bcout_t *bco);

/******************************************************************************/

int bco_w0(bcout_t *bco, bc_t bc);
int bco_w1(bcout_t *bco, bc_t bc, uint8_t arg);
int bco_w2(bcout_t *bco, bc_t bc, uint8_t arg0, uint8_t arg1);
int bco_int(bcout_t *bco, int i);
int bco_str(bcout_t *bco, const char *str);
int bco_find_int(bcout_t *bco, int i);
int bco_find_str(bcout_t *bco, const char *str);

/******************************************************************************/

#endif /* COMPILER_BCOUT_H_ */
