CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Og -ggdb3 -std=c99
OBJS=\
	memory.o \
	k_array.o \
	k_string.o \
	loader.o \
	vm.o \
	stack.o \
	main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJS): stack.h vm.h types.h bc.h loader.h k_array.h memory.h

clean:
	rm $(OBJS)

clobber: clean
	rm kek
