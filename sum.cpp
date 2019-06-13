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
void check_equal(const T * v1, const T * v2, const int num ) {
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
    #if defined LITTEL_CASE2
        PageRankStructure page_rank_structure = little_test2();
        PageRankStructurePtr  page_rank_structure_ptr = & page_rank_structure;
    #else
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        PageRankStructurePtr page_rank_structure_ptr = test_read( argv[1]);
    #endif
   
    PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
    float * sum = page_rank_structure_ptr->sum;

    float * rank = page_rank_structure_ptr->rank;

    int * n1 = page_rank_structure_ptr->n1;

    int * n2 = page_rank_structure_ptr->n2;
    int * nneibor = page_rank_structure_ptr->nneibor;
    int nedges = page_rank_structure_ptr->nedges;
    int nnodes = page_rank_structure_ptr->nnodes;
    float * sum_bak = SIMPLE_MALLOC( float, nnodes );

    float * sum_time = SIMPLE_MALLOC( float, nnodes );


    int nedges_pack_num = nedges / VECTOR;
    ShuffleIndexPtr shuffle_index_ptr = (ShuffleIndexPtr) _mm_malloc(sizeof(ShuffleIndex)*4*nedges_pack_num,64);

    int *  mask_vec = (int*)malloc(sizeof(int)*nedges_pack_num);
    for(int j=0 , j_pack = 0 ;j_pack < nedges_pack_num ;j_pack++, j+=VECTOR) {
        int *ny = &n2[j];
        int num = analyze( ny , &shuffle_index_ptr[j_pack*4]);
        mask_vec[j_pack] = num;
    }

    LOG(INFO) << "Before Make\n";
    fflush(stdout);

    page_rank_statement_ptr->make( page_rank_structure_ptr, mask_vec );
    LOG(INFO) << "After Make\n";
    fflush(stdout);
    LOG(INFO) << "Before GetFunction\n";
    fflush(stdout);

    FuncStatement * func_ptr = page_rank_statement_ptr->get_function(); 
    StateMentPrint statement_print;
    LOG(INFO) << "After GetFunction\n";
    fflush(stdout);

//    statement_print.print( func_ptr->get_state() ,std::cout);
    LOG(INFO) << "Before AddFunction\n";
    fflush(stdout);

    LLVMCodeGen codegen;
    codegen.AddFunction( func_ptr );
    LOG(INFO) << "After AddFunction\n";

//    codegen.PrintModule();
    LLVMModule * llvm_module_ptr = new LLVMModule( codegen.get_mod(),codegen.get_ctx() );
     LOG(INFO) << "After New module\n";
    fflush(stdout);

    llvm_module_ptr->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");
     LOG(INFO) << "After Init module\n";
    fflush(stdout);
   
//    LLVMLOG(INFO) << * (llvm_module_ptr->get_module());
    //using func = void( double*,double*,double*,int* );
//    std::cout << llvm_module_ptr->GetSource("asm");
    LOG(INFO) << "Before Get Function\n";
    fflush(stdout);
    BackendPackedCFunc func = llvm_module_ptr->GetFunction("function");
    LOG(INFO) << "After Get Function\n";
    fflush(stdout);

    //    printf("\ny_array \n");

        for( int i = 0 ; i < nnodes ; i++ )
        sum_bak[i] = sum[i];
    for( int i = 0 ; i < nnodes ; i++ )
        sum_time[i] = sum[i];

//    print_vec( n1, nedges );

//    print_vec( n2, nedges );

    LOG(INFO) << "Before Calc\n";

    fflush(stdout);
    Timer::startTimer("aot");
    for(int j=0;j<nedges;j++) {
      int nx = n1[j];
      int ny = n2[j];
      sum_bak[ny] += rank[nx] / nneibor[nx];
    }
    Timer::endTimer("aot");

    Timer::printTimer("aot");

    LOG(INFO) << "After Calc\n";

    fflush(stdout);
//    print_vec( sum_bak, nnodes );    
//    fflush(stdout);

//    print_vec( sum, nnodes );    
//    print_vec(y_array_bak,row_num);
//    for( int i = 0 ; i < 50 ; i++ )
//        func(       y_array_time,     x_array,data_ptr,column_ptr, analyze_CSR5.get_tile_row_index_ptr()); 

    LOG(INFO) << "Before Calc\n";

    fflush(stdout);

    func( sum,n1,n2,rank,nneibor,(char*)shuffle_index_ptr );
    for( int i = 0 ; i < 50 ; i++ )
        func( sum_time,n1,n2,rank,nneibor ,(char*)shuffle_index_ptr);
    Timer::startTimer("jit");
//     for( int i = 0 ; i < 100 ; i++ )
//        func(       y_array_time,     x_array,data_ptr,column_ptr, analyze_CSR5.get_tile_row_index_ptr()); 
    for( int i = 0 ; i < 1000; i++ )
        func( sum_time,n1,n2,rank,nneibor ,(char*)shuffle_index_ptr);

    Timer::endTimer("jit");
    Timer::printTimer("jit",1000);

    LOG(INFO) << "After Calc\n";
//    print_vec(y_array,row_num); 
    check_equal( sum,sum_bak,nnodes );
    return 0;
}
