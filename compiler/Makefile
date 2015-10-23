CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -Wno-write-strings -Wno-unused-variable -O0 -ggdb3

OBJS=\
	lexan.o \
	main.o \
	parser.o \
	strom.o \
	tabsym.o \
	vstup.o \
	zaspoc.o

scheme: $(OBJS)
	$(CXX) -o main $(OBJS)

$(OBJS): lexan.h parser.h strom.h tabsym.h vstup.h zaspoc.h

clean:
	rm $(OBJS)

clobber: clean
	rm main
