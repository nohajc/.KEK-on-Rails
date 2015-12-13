CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Og -ggdb3 -std=c99 -Wno-unused-variable -Wno-unused-label
LDFLAGS=-lreadline
OBJS=\
	memory.o \
	k_integer.o \
	k_array.o \
	k_string.o \
	k_symbol.o \
	k_file.o \
	k_term.o \
	k_sys.o \
	k_exception.o \
	k_meta.o \
	loader.o \
	stack.o \
	vm.o \
	main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): stack.h vm.h types.h bc.h loader.h k_integer.h k_array.h k_string.h k_symbol.h k_file.h k_term.h k_sys.h k_exception.h k_meta.h memory.h

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f kek
