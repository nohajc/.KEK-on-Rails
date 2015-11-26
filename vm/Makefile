CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Og -ggdb3 -std=c99 -Wno-unused-variable -Wunused-label
LDFLAGS=-lreadline
OBJS=\
	memory.o \
	k_integer.o \
	k_array.o \
	k_string.o \
	k_file.o \
	k_term.o \
	k_sys.o \
	k_exception.o \
	loader.o \
	stack.o \
	vm.o \
	main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(OBJS): stack.h vm.h types.h bc.h loader.h k_integer.h k_array.h k_string.h k_file.h k_term.h k_sys.h k_exception.h memory.h

clean:
	rm $(OBJS)

clobber: clean
	rm kek
