/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include "llvm_lib/llvm_module.h"
#include <map>
#include "llvm/IR/InstrTypes.h"

#include "llvm/IR/Verifier.h"
#include <cstdlib>
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DIBuilder.h>


/////////////////////////////
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include "csr_matrix.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "csr_matrix.h"
#include "Timers.hpp"
#include <sstream>
#include "statement.hpp"
#include "csr5_statement.hpp"
#include "statement_print.hpp"
#include "small_case.hpp"
#include "llvm_lib/llvm_codegen.hpp"
#include "analyze.h"
#include "pagerank_fuse_all.hpp"
#define PRINTINT(x) do {    \
                    printf( #x" %d\n" , (x));fflush(stdout); \
                        } while(0)
void spmv_local(DATATYPE * y_ptr,const DATATYPE * x_ptr,const DATATYPE * data_ptr, const int * column_ptr, const int * row_ptr, const int row_num ) {
    for (int i = 0 ; i < row_num ; i++ ) {
        DATATYPE sum = 0;
        for (int j = row_ptr[i]; j < row_ptr[i+1] ; j++ ) {
            sum += x_ptr[column_ptr[j]] * data_ptr[j];
        }
        y_ptr[i] += sum;
    }
}

void init_vec(DATATYPE * dence_vec_ptr, const int data_num , const DATATYPE data, const bool change = false ) {
    if( change ) {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = i;
        }
    } else {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = data;
        }
    }
}
template<typename T>
bool check_equal(const T * v1, const T * v2, const int num ) {
    bool flag = true;
    for( int i = 0 ; i < num ; i++ ) {
        if( (v1[i]-v2[i]) > 1e-3 || (v2[i]-v1[i]) > 1e-3 ) {
            flag = false;
            std::cout<< i<< " "  << v1[i]  << " "<< v2[i]<<"\n";
        }
    }
    if(flag) 
        std::cout<<"Correct"<<std::endl;
    else 
        std::cout<<"False"<<std::endl;
    return flag;
}

template<typename T>
void print_vec( T * data_ptr, const int num ) {
    for( int i = 0 ; i < num ; i++ ) {
        std::cout<< data_ptr[i] << " ";
    }
    std::cout<<std::endl;
}
//#define LITTEL_CASE2
int main( int argc , char const * argv[] ) {
    #ifdef LITTEL_CASE
        csrSparseMatrix sparseMatrix = little_test();
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #elif defined LITTEL_CASE2

        csrSparseMatrix sparseMatrix = little_test2(1024 ,1024);
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #else
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        csrSparseMatrixPtr sparseMatrixPtr = matrix_read_csr( argv[1]);
    #endif
  
    
    DATATYPE * data_ptr = sparseMatrixPtr->data_ptr;
    int * column_ptr = sparseMatrixPtr->column_ptr;
    int * row_ptr = sparseMatrixPtr->row_ptr;

    const int data_num = sparseMatrixPtr->data_num;
    const int row_num = sparseMatrixPtr->row_num;
    const int column_num = sparseMatrixPtr->column_num;
    DATATYPE * x_array = SIMPLE_MALLOC( DATATYPE , column_num );
    DATATYPE * y_array = SIMPLE_MALLOC( DATATYPE, row_num );
    DATATYPE * y_array_bak = SIMPLE_MALLOC( DATATYPE , row_num );
    
    DATATYPE * y_array_time = SIMPLE_MALLOC( DATATYPE, row_num );
    init_vec(x_array,column_num,1);
//    init_vec( x_array, column_num , 1 ,true);
    init_vec( y_array, row_num , 0 );
    init_vec( y_array_bak, row_num , 0 );

    init_vec( y_array_time, row_num , 0 );


    PageRankFuseAll page_rank_fuse_all;

    page_rank_fuse_all.compiler( row_ptr, column_ptr, row_num, column_num  );

    Timer::startTimer("aot");
        spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );

    Timer::endTimer("aot");

    Timer::printTimer("aot");
    page_rank_fuse_all( y_array, x_array,data_ptr,column_ptr,row_ptr,row_num );

     for( int i = 0 ; i < 50 ; i++ )
        page_rank_fuse_all( y_array_time, x_array,data_ptr,column_ptr,row_ptr,row_num );

#define TIMES 100
    Timer::startTimer("jit");
     for( int i = 0 ; i < TIMES ; i++ )
        page_rank_fuse_all( y_array_time, x_array,data_ptr,column_ptr,row_ptr,row_num );

    Timer::endTimer("jit");
    Timer::printTimer("jit",TIMES);
    if(!check_equal( y_array_bak, y_array, row_num )) {
        return 1;
    }

    return 0;
}
