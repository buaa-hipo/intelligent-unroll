CC=g++
#CFLAGS=-O3 -ffast-math `llvm-config --cflags`  

CFLAGS=-std=c++11 -O3 -ffast-math  `llvm-config --cxxflags` -frtti 
#CFLAGS=-g -ffast-math `llvm-config --cflags`
LD=g++ 
LDFLAGS= -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`   -rdynamic -llzma -std=c++11
 

all: sum
LKFILE= sum.o Timers.o llvm_common.o util.o llvm_codegen.o type.o statement.o statement_print.o csr_matrix.o csr5_statement.o
sum: $(LKFILE)
	$(LD) $^ $(LDFLAGS) -o $@

sum.o: sum.cpp analyze_csr5.hpp statement.hpp csr5_statement.hpp type.hpp statement_print.hpp small_case.hpp ir_func.hpp log.h llvm_lib/llvm_codegen.hpp llvm_lib/llvm_module.h llvm_lib/llvm_print.hpp
	$(CC) $(CFLAGS) -c $<
#	$(CC) $< -S $(CFLAGS) -o sum.S
Timers.o:Timers.cpp
	$(CC) $< -c -o $@
#csr_matrix.o:csr_matrix.cpp
#	$(CC) $< -c -o $@
util.o:util.cpp
	$(CC) $< -c -o $@

llvm_codegen.o:llvm_lib/llvm_codegen.cpp llvm_lib/llvm_codegen.hpp
	$(CC) $(CFLAGS) $< -c -o $@
llvm_common.o:llvm_lib/llvm_common.cpp
	$(CC) $(CFLAGS) $< -c -o $@

type.o:type.cpp type.hpp
	$(CC) $(CFLAGS) $< -c -o $@
statement.o:statement.cpp
	$(CC) $(CFLAGS) $< -c -o $@

csr_matrix.o:csr_matrix.cpp
	$(CC) $(CFLAGS) $< -c -o $@

statement_print.o:statement_print.cpp
	$(CC) $(CFLAGS) $< -c -o $@

csr5_statement.o:csr5_statement.cpp
	$(CC) -std=c++11  $< -c -o $@
sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

clean:
	-rm -f sum.o sum sum.bc sum.ll *.o
#~/llvm-project-llvmorg-7.0.1/build/bin/llc -mcpu=x86-64 -mattr=+fma,+avx2,avx,sse,sse2 sum.bc -o sum.S
