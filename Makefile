# CC=g++
CC=icpc
#CFLAGS=-O3 -ffast-math `llvm-config --cflags` 
# ARCH=core-avx2
# ARCH=knl
INCLUDE=-Iparse/ -Inode/ -Ilog/ -Itype/ -Ihash/ -Itools_set/ -Istatement/ -Ipass/ -Ibit2addr/ -Iutil/ -Iio_matrix/ -ITimer/ -Itransform_data/ -Iintelligent_unroll/ -I./include/ -I./
# CFLAGS=-march=$(ARCH) -std=c++11 -O2 -ffast-math  -frtti -Wall -Werror

# CFLAGSPermitWarning=-std=c++11 -O2 -ffast-math  -frtti -Wall 

CFLAGS=-xHost -std=c++11 -O3 -ffast-math  -frtti -Wall -Werror
CFLAGSPermitWarning=-std=c++11 -xHost -O3 -ffast-math  -frtti -Wall 

INCLUDE_LLVM_DIR=`llvm-config --cxxflags` 
#CFLAGS=-g -ffast-math `llvm-config --cflags`
#LD=g++ 
LD=icpc
LDFLAGS= -lpapi -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`   -rdynamic -llzma -std=c++11
LINK_DIR=build/ 

AR=ar

all: libdynvec spmv pagerank
LKFILE= type.o parse.o tools_set.o transform_data.o node2state.o pass.o state_formulation.o state_redirect_var.o state_optimization.o intelligent_unroll.o llvm_common.o util.o llvm_codegen.o statement.o statement_print.o csr_matrix.o bit2addr.o tools_set.o Timers.o mmio.o 
libdynvec: $(LKFILE)
	$(AR) -rvs libdynvec.a $^

pagerank: $(LKFILE) pagerank.o
	$(LD) $^ $(LDFLAGS) -o $@

spmv: $(LKFILE) spmv.o
	$(LD) $^ $(LDFLAGS) -o $@
parse.o:parse/parse.cpp node/node.hpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
tools_set.o:tools_set/tools_set.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
transform_data.o:transform_data/transform_data.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
node2state.o:node/node2state.cpp node/node.hpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
pass.o:pass/state_pass.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
state_formulation.o:pass/state_formulation.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@

state_optimization.o:pass/state_optimization.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@

state_redirect_var.o:pass/state_redirect_var.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
intelligent_unroll.o:intelligent_unroll/intelligent_unroll.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
llvm_common.o:llvm_lib/llvm_common.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $(INCLUDE_LLVM_DIR) $< -o $@
util.o:util/util.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $(INCLUDE_LLVM_DIR) $< -o $@
llvm_codegen.o:llvm_lib/llvm_codegen.cpp 
	$(CC) -c $(INCLUDE) $(INCLUDE_LLVM_DIR)  $(CFLAGS) $< -o $@
type.o:type/type.cpp 
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
statement.o:statement/statement.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
statement_print.o:statement/statement_print.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
csr_matrix.o:io_matrix/csr_matrix.cpp
	$(CC) -c $(INCLUDE) $(CFLAGSPermitWarning) $< -o $@
mmio.o:io_matrix/mmio.c
	$(CC) -c $(INCLUDE) $(CFLAGSPermitWarning) $< -o $@

Timers.o:Timer/Timers.cpp
	$(CC) $< -c -o $@
spmv.o: app/spmv.cpp 
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
pagerank.o: app/pagerank.cpp 
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@

#	$(CC) $< -S $(CFLAGS) -o sum.S
#csr_matrix.o:csr_matrix.cpp
#	$(CC) $< -c -o $@



bit2addr.o:bit2addr/bit2addr.cpp
	$(CC) -c $(INCLUDE) $(CFLAGS) $< -o $@
sum.bc: sum
	./sum 0 0

sum.ll: sum.bc
	llvm-dis $<


clean:
	-rm -f sum.o sum sum.bc sum.ll *.o
