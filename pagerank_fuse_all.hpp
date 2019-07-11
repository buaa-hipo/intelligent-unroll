#ifndef PAGERANK_FUNCTION_HPP
#define PAGERANK_FUNCTION_HPP
#include "unroll_function.hpp"
using FType1 = void( int*,int*,const int  , const int );
using FType2 = int( double*,double*,double*,int*,int*,int );
using FType3 = void( int**,int** );
    //int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
using UnrollFunctionSpec = UnrollFunction<FType1,FType2,FType3>;
typedef int(*inner_func)( double*, double*, double*,int*,int*,int,int**,int**);

class PageRankFuseAll : public UnrollFunctionSpec {
    int mask_num_;
    int ** index_ptr_ptr_;
    int ** row_ptr_ptr_;
    std::map<Mask2, std::pair<std::vector<int>,std::vector<int> >> mask_map_;
    std::map<Mask2, int> mask_num_map_;

    public:
    void analyze(int * row_ptr, int * column_ptr, const int row_num , const int column_num ) {
        Analyze( mask_map_,mask_num_map_,mask_num_, row_ptr, column_ptr,row_num, column_num);
        index_ptr_ptr_ = (int**)malloc(sizeof( int*) * mask_num_);
        row_ptr_ptr_ = ( int** )malloc(sizeof( int*) * mask_num_);
        int i = 0 ;
        for( const auto & it : mask_num_map_ ) {
             index_ptr_ptr_[i] = (int*)malloc(sizeof(int)*it.second );
             row_ptr_ptr_[i] = (int*)malloc(sizeof(int)*it.second);
             i++;
        }
        i = 0 ;
        for( const auto & it : mask_map_ ) {
            const std::vector<int> & row_begin_vec = it.second.first;

            const std::vector<int> & index_vec = it.second.second;
            int j = 0 ;
            for( auto it : row_begin_vec ) {
                row_ptr_ptr_[i][j] = it;
                j++;
            }
            int jj = 0 ;
            for( auto it : index_vec) {
                index_ptr_ptr_[i][jj] = it;
                jj++;
            }
            i++;
        }
    }

    void generate_func(  ) {
        PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
        page_rank_statement_ptr->make( mask_num_map_ );
        func_ptr_ = page_rank_statement_ptr->get_function();
    //    std::cout<<func_ptr_->get_state();
    }
     
    int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
       (*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, index_ptr_ptr_ );
    }
};


#endif
