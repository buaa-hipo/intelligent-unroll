CC=g++
#CFLAGS=-O3 -ffast-math `llvm-config --cflags`  

CFLAGS=-std=c++11 -O3 -ffast-math  `llvm-config --cxxflags` -frtti -Ibit2addr/ 
#CFLAGS=-g -ffast-math `llvm-config --cflags`
LD=g++ 
LDFLAGS= -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`   -rdynamic -llzma -std=c++11
 

all: sum
LKFILE= Timers.o llvm_common.o util.o llvm_codegen.o type.o statement.o statement_print.o csr_matrix.o csr5_statement.o analyze.o bit2addr.o pagerank_fuse_all.o element.o sum.o 
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

llvm_codegen.o:llvm_lib/llvm_codegen.cpp llvm_lib/llvm_codegen.hpp statement.hpp
	$(CC) $(CFLAGS) $< -c -o $@
llvm_common.o:llvm_lib/llvm_common.cpp
	$(CC) $(CFLAGS) $< -c -o $@

type.o:type.cpp type.hpp
	$(CC) $(CFLAGS) $< -c -o $@
statement.o:statement.cpp statement.hpp
	$(CC) $(CFLAGS) $< -c -o $@

csr_matrix.o:csr_matrix.cpp
	$(CC) $(CFLAGS) $< -c -o $@

element.o:element.cpp
	$(CC) -Ibit2addr/ -std=c++11  $< -c -o $@
statement_print.o:statement_print.cpp statement.hpp
	$(CC) $(CFLAGS) $< -c -o $@

csr5_statement.o:csr5_statement.cpp statement.hpp
	$(CC) -Ibit2addr/ -std=c++11  $< -c -o $@

bit2addr.o:bit2addr/bit2addr.cpp
	$(CC) -std=c++11  $< -c -o $@
sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<

pagerank_fuse_all.o:pagerank_fuse_all.cpp
	$(CC) -std=c++11 -O3 -I./bit2addr  $< -c -o $@
analyze.o:analyze.cpp
	$(CC) -std=c++11 -O3  $< -c -o $@
clean:
	-rm -f sum.o sum sum.bc sum.ll *.o
#~/llvm-project-llvmorg-7.0.1/build/bin/llc -mcpu=x86-64 -mattr=+fma,+avx2,avx,sse,sse2 sum.bc -o sum.S
