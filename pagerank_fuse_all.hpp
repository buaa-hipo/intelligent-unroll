#ifndef PAGERANK_FUNCTION_HPP
#define PAGERANK_FUNCTION_HPP
#include "unroll_function.hpp"
using FType1 = void( ShuffleIndexPtr, int*,int,const int * ,int* );
using FType2 = int( float*,int*,int*,float*,int*,char* );
using UnrollFunctionSpec = UnrollFunction<FType1,FType2,char>;
typedef int(*inner_func)( int**,float*,int*,int*,float*,int*,char*);

class PageRankFuseAll : public UnrollFunctionSpec {
    int * mask_vec_;
    int nedges_pack_num_;
    int * addr_num_;
    int ** addr_;
    public:
    void analyze( ShuffleIndexPtr shuffle_index_ptr, int * mask_vec, int nedges_pack_num , const int * n2 , int * addr_num ) { 
        int ** addr = Analyze( shuffle_index_ptr, mask_vec, nedges_pack_num ,n2, addr_num);
        mask_vec_ = mask_vec;
        nedges_pack_num_ = nedges_pack_num;
        addr_num_ = addr_num;
        addr_ = addr;
    }

    void generate_func(  ) {
        PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
        page_rank_statement_ptr->make(  mask_vec_, nedges_pack_num_, addr_num_ );
        func_ptr_ = page_rank_statement_ptr->get_function();
    }
     
    int operator() ( float* sum ,int*n1,int*n2,float*rank,int*nneibor,char*shuffle_index_ptr) {
    
       inner_func func = llvm_module_ptr_->GetFunction("function");
    
       (*func)(addr_, sum,n1,n2,rank,nneibor,(char*)shuffle_index_ptr );
    }
};


#endif
