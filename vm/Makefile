CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wno-write-strings -Wno-unused-variable -Og -ggdb3 
OBJS=\
	stack.o \
	vm.o \
    main.o

kek: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJS): stack.h vm.h

clean:
	rm $(OBJS)

clobber: clean
	rm kek