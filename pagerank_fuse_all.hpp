#ifndef PAGERANK_FUNCTION_HPP
#define PAGERANK_FUNCTION_HPP
#include "analyze.h"
#include "statement.hpp"
#include "unroll_function.hpp"
#include "llvm_lib/llvm_module.h"
using FType1 = void( int*,int*,const int  , const int,double* );
using FType2 = int( double*,double*,double*,int*,int*,int );
using FType3 = void( int**,int** ,double**,int64_t**,int**);
    //int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
using UnrollFunctionSpec = UnrollFunction<FType1,FType2,FType3>;
//typedef int(*inner_func)( double*, double*, double*,int*,int*,int,int**,int**,double**);

class PageRankFuseAll : public UnrollFunctionSpec {
    int mask_num_;
    int ** index_ptr_ptr_;
    int ** reduce_index_mask_ptr_ptr_;
    int ** load1_to_gather_num_ptr_ptr_;
    int64_t ** load1_to_gather_addr_ptr_ptr_; 
    int ** load1_to_gather_column_begin_ptr_ptr_;
    int ** row_ptr_ptr_;
    int ** column_ptr_ptr_;
    double ** data_ptr_ptr_;
    int * num_ptr_;
    int * mask_ptr_;
    int * inner_num_ptr_;
    std::map<Mask2, std::pair<std::vector<int>,std::vector<int> >> mask_map_;
    std::map<Mask2, int> mask_num_map_;
    void generate_index_and_row_ptr_ptr();

    int analyze_gather_addr(const int * column_block_ptr, int64_t &addr,int & column_begin) ;
    void transform(double * m_data_ptr, int * m_col_ptr) ;

    void generate_gather_mask(int * m_col_ptr) ;
    public:

    
    void analyze(int * row_ptr, int * column_ptr, const int row_num ,const int column_num ,double*data_ptr) ;
    void generate_func(  ) ;
     
    int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
       (*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, column_ptr_ptr_, data_ptr_ptr_,load1_to_gather_addr_ptr_ptr_,load1_to_gather_num_ptr_ptr_ );
    }
};


#endif
