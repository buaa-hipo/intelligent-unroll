CC=clang++
#CFLAGS=-O3 -ffast-math `llvm-config --cflags`

CFLAGS=-g -ffast-math `llvm-config --cflags`
LD=clang++
LDFLAGS= -ffast-math `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`

all: sum

sum.o: sum.cpp
	$(CC) $(CFLAGS) -c $<
#	$(CC) $< -S $(CFLAGS) -o sum.S
sum: sum.o
	$(LD) $< $(LDFLAGS) -o $@

sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

clean:
	-rm -f sum.o sum sum.bc sum.ll
