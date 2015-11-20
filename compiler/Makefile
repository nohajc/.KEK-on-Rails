CXX=g++
CXXFLAGS=-Wall -Wextra -pedantic -Wno-write-strings -Wno-unused-variable -Wno-unused-parameter -Wno-long-long -O0 -ggdb3

OBJS=\
	bcout.o \
	lexan.o \
	kekc.o \
	parser.o \
	strom.o \
	tabsym.o \
	vstup.o \

kekc: $(OBJS)
	$(CXX) -o $@ $(OBJS)

$(OBJS): lexan.h parser.h strom.h tabsym.h vstup.h bcout.h

clean:
	rm $(OBJS)

clobber: clean
	rm main
