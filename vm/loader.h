/*
 * loader.h
 *
 *  Created on: Nov 7, 2015
 *      Author: n
 */

#ifndef VM_LOADER_H_
#define VM_LOADER_H_

typedef struct _kexe {
	int classes;

} kexe_t;

kexe_t *kexe_load(const char *filename);

#endif /* VM_LOADER_H_ */
