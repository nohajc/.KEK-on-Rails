CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Og -ggdb3 -std=c99
OBJS=\
	loader.o \
	vm.o \
	stack.o \
	main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJS): stack.h vm.h

clean:
	rm $(OBJS)

clobber: clean
	rm kek
