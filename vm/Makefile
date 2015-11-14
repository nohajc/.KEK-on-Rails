CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Og -ggdb3 -std=c99 -Wno-unused-variable
OBJS=\
	memory.o \
	k_array.o \
	k_string.o \
	loader.o \
	stack.o \
	vm.o \
	main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJS): stack.h vm.h types.h bc.h loader.h k_array.h k_string.h memory.h

clean:
	rm $(OBJS)

clobber: clean
	rm kek
