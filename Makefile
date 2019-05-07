CC=clang++
CFLAGS=-O3 -ffast-math `llvm-config --cflags` -g 

#CFLAGS=-g -ffast-math `llvm-config --cflags`
LD=clang++ 
LDFLAGS= -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs` -lLLVMDebugTool  -rdynamic -g

all: sum
LKFILE= sum.o Timers.o csr_matrix.o
sum: $(LKFILE)
	$(LD) $^ $(LDFLAGS) -o $@

sum.o: sum.cpp
	$(CC) $(CFLAGS) -c $<
#	$(CC) $< -S $(CFLAGS) -o sum.S
Timers.o:Timers.cpp
	$(CC) $< -c -o $@
csr_matrix.o:csr_matrix.cpp
	$(CC) $< -c -o $@
sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

clean:
	-rm -f sum.o sum sum.bc sum.ll *.o
