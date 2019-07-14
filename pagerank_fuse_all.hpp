#ifndef PAGERANK_FUNCTION_HPP
#define PAGERANK_FUNCTION_HPP
#include "unroll_function.hpp"
using FType1 = void( int*,int*,const int  , const int,double* );
using FType2 = int( double*,double*,double*,int*,int*,int );
using FType3 = void( int**,int** ,double**);
    //int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
using UnrollFunctionSpec = UnrollFunction<FType1,FType2,FType3>;
//typedef int(*inner_func)( double*, double*, double*,int*,int*,int,int**,int**,double**);

class PageRankFuseAll : public UnrollFunctionSpec {
    int mask_num_;
    int ** index_ptr_ptr_;
    int ** row_ptr_ptr_;
    int ** column_ptr_ptr_;
    double ** data_ptr_ptr_;
    int * num_ptr_;
    int * mask_ptr_;
    int * inner_num_ptr_;
    std::map<Mask2, std::pair<std::vector<int>,std::vector<int> >> mask_map_;
    std::map<Mask2, int> mask_num_map_;
    void generate_index_and_row_ptr_ptr( ) {
        index_ptr_ptr_ = (int**)malloc(sizeof( int*) * mask_num_);
        row_ptr_ptr_ = ( int** )malloc(sizeof( int*) * mask_num_);
        num_ptr_ = (int*) malloc(sizeof(int)*mask_num_);
        mask_ptr_ = (int*) malloc(sizeof(int)*mask_num_);
        inner_num_ptr_ = (int*)malloc(sizeof(int)*mask_num_);
        int i = 0 ;
        for( const auto & it : mask_num_map_ ) {
             index_ptr_ptr_[i] = (int*)malloc(sizeof(int)*it.second );
             row_ptr_ptr_[i] = (int*)_mm_malloc(sizeof(int)*it.second,ANONYMOUSLIB_X86_CACHELINE);
             inner_num_ptr_[i] = it.first.num_;
             num_ptr_[i] = it.second;
             mask_ptr_[i] = it.first.mask_;
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
    void transform(double * m_data_ptr, int * m_col_ptr) {
        column_ptr_ptr_ = ( int ** )malloc(sizeof(int*)*mask_num_);
        data_ptr_ptr_ = (double**)malloc(sizeof(double*)*mask_num_);
        ////malloc
         for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             int * column_ptr = (int*)_mm_malloc(sizeof(int)* mask_num * inner_mask_num*VECTOR,ANONYMOUSLIB_X86_CACHELINE);
             double * data_ptr = (double*)_mm_malloc(sizeof(double)* mask_num * inner_mask_num*VECTOR,ANONYMOUSLIB_X86_CACHELINE);
             if( column_ptr == NULL || data_ptr == NULL ) {
                LOG(FATAL) << "malloc fatal";
             } else {
             
                column_ptr_ptr_[i] = column_ptr;
                data_ptr_ptr_[i] = data_ptr;
             }
         }
        
        LOG(INFO) << "after malloc";
        /////transfer
        for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             const int mask = mask_ptr_[i];
             int * column_ptr = column_ptr_ptr_[i];
             double * data_ptr = data_ptr_ptr_[i];
             int * index_ptr = index_ptr_ptr_[i];
             int j = 0 ,jj;
             if( mask != 0x1 ) {
                 CHECK( inner_mask_num == 1 ) << "inner mask num should be 1\n";
                 if( j <= mask_num - VECTOR ) {
                 for(  jj =0 ; j <= mask_num - VECTOR ;jj++, j += VECTOR) {
                     int * column_block_ptr = &column_ptr[j*VECTOR];
                     double * data_block_ptr = &data_ptr[j*VECTOR];
                     int * index_block_ptr = &index_ptr[jj];
                           for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                                const int index = index_block_ptr[v_i]; 
                                for( int w = 0 ; w < VECTOR ; w++ ) {
                                    column_block_ptr[ w * VECTOR + v_i ] = m_col_ptr[ index * VECTOR + w ];
                                    data_block_ptr[w * VECTOR + v_i] = m_data_ptr[ index * VECTOR + w ];
                                }
                           }
                 }
                 j -= VECTOR;
                 }
                
             }
            /// j is the problem
             for(  ; j < mask_num ; j++ ) {
                const int index = index_ptr[j];
                for( int inner_i = 0 ; inner_i < inner_mask_num ; inner_i++ ) {
                    int * column_block_ptr = &column_ptr[j*VECTOR * inner_mask_num + inner_i * VECTOR ];
                    double * data_block_ptr = &data_ptr[j*VECTOR *inner_mask_num + inner_i * VECTOR ];

                    for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                        column_block_ptr[v_i] = m_col_ptr[ (index + inner_i) * VECTOR + v_i ];
                        data_block_ptr[v_i] = m_data_ptr[(index + inner_i) * VECTOR + v_i];
                    }
                }
             }
        }
    }
    public:

    void analyze(int * row_ptr, int * column_ptr, const int row_num ,const int column_num ,double*data_ptr) {
        Analyze( mask_map_,mask_num_map_,mask_num_, row_ptr, column_ptr,row_num, column_num);
        generate_index_and_row_ptr_ptr();
        LOG(INFO) << "Before Trans";
        transform( data_ptr, column_ptr);
        LOG(INFO) << "After Trans";

    }

    void generate_func(  ) {
        PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
        page_rank_statement_ptr->make( mask_num_map_ );
        func_ptr_ = page_rank_statement_ptr->get_function();
        std::cout<<func_ptr_->get_state();
    }
     
    int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
       (*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, column_ptr_ptr_, data_ptr_ptr_ );
    }
};


#endif
