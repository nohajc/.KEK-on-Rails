CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -Wno-write-strings -Wno-unused-variable -Wno-long-long -O0 -ggdb3

OBJS=\
	bcout.o \
	lexan.o \
	main.o \
	parser.o \
	strom.o \
	tabsym.o \
	vstup.o \

scheme: $(OBJS)
	$(CXX) -o main $(OBJS)

$(OBJS): lexan.h parser.h strom.h tabsym.h vstup.h bcout.h

clean:
	rm $(OBJS)

clobber: clean
	rm main
