/*
 * n{esrotom,ohajan}@fit.cvut.cz
 * 2015
 * https://github.com/nohajc/.KEK-on-Rails
 */

#include <stdio.h>
#include <assert.h>
#include "vm.h"

void program_load(const char *filename) {
	printf("%s\n", filename);
}

class_t *class_find(const char *key) {
	printf("%s\n", key);
	return (NULL);
}

method_t *method_find(class_t *class, const char *key) {
	printf("%s->%s\n", class->name, key);
	return (NULL);
}

void method_eval(method_t *method) {
	printf("%s\n", method->name);
}

class_t *class_new(const char *name) {
	class_t *c;

	c = malloc(sizeof (class_t));
	assert(c);

	c->name = name;

	return (c);
}

void class_free(class_t *class) {
	free(class);
}
