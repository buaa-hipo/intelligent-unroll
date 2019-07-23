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
    int ** reduce_load1_to_gather_num_ptr_ptr_;
    int ** reduce_index_inner_ptr_ptr_; 
    int64_t ** reduce_load1_to_gather_addr_ptr_ptr_; 

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
#define LOAD_TO_GATHER_LEVEL 1
    typedef union GatherAddr {
        int64_t data;
        char data_vec[VECTOR];
    } GatherAddr;
    int analyze_gather_addr(const int * column_block_ptr, int64_t &addr,int & column_begin) {
        GatherMask gather_addr;
        std::vector<int> column_begin_vec;
        int column_block[VECTOR];
        for( int v_i = 0 ; v_i < VECTOR; v_i++ ) {
            column_block[v_i] = column_block_ptr[v_i];
        }
        int num = 0;
        while(true) {
            int min = INT_MAX ;
            for( int v_i = 1 ; v_i < VECTOR ; v_i++) {
                min = column_block[v_i] < min ? column_block[v_i] : min;
            }
            if( min == INT_MAX) {
                break;
            } else {
                column_begin_vec.push_back( min );
            }
            num++;
            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                if( column_block[v_i] < min + VECTOR ) {
                    column_block[v_i] = INT_MAX ;
                    gather_addr.data_vec[ v_i ] = column_block[v_i] - min;
                }
            }
        }
        if( num <= LOAD_TO_GATHER_LEVEL ) {
            addr = gather_addr.data;
            column_begin = column_begin_vec[0];
            return num;
        } else {
            return -1;
        }
    }

    void generate_gather_mask(int * m_col_ptr) {
        
        reduce_load1_to_gather_num_ptr_ptr_ = (int**)malloc(sizeof(int*)*mask_num_);
        reduce_load1_to_gather_addr_ptr_ptr_ = ( int64_t** )malloc(sizeof(int64_t*)*mask_num_);
        reduce_load1_to_gather_column_begin_ptr_ptr_ = (int**)malloc(sizeof(int*)*mask_num_);
        load1_gather_num_ptr_ = (int*)malloc( sizeof( int ) * mask_num_);
        for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_has_data_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             const int mask = mask_ptr_[i];
             int * index_ptr = index_ptr_ptr_[i];
             std::vector<int64_t> load1_gather_addr_vec;

             std::vector<int> load1_gather_index_vec;

             std::vector<int> load1_gather_column_begin_vec;

             std::vector<int> load_gather_index_vec;
//             std::vector<int> load1_gather_addr_num_vec;

             if( mask == 0x1 ) {
                 int * reduce_load1_to_gather_num_ptr = (int*)malloc(sizeof(int)*mask_has_data_num);
                 reduce_load1_to_gather_num_ptr_ptr_[i] = reduce_load1_to_gather_num_ptr;
                 int load1_gather_addr_num_sum = 0;

                 int * reduce_index_inner_ptr = (int*)malloc(sizeof(int)* inner_mask_num * mask_has_data_num);
                 reduce_index_inner_ptr_ptr_[i] = reduce_index_inner_ptr;

                 for(int j = 0  ; j < mask_has_data_num ; j++ ) {
                    const int index = index_ptr[j];
                    int load1_gather_addr_num = 0;
                    int * reduce_index_inner_ptr_j = reduce_index_inner_ptr + j * inner_mask_num;
                    for( int inner_i = 0 ; inner_i < inner_mask_num ; inner_i++ ) {

                        int * column_block_ptr = m_col_ptr[ (index + inner_i) * VECTOR  ];
                        int64_t load1_gather_addr;
                        int  column_begin;
                        int num = analyze_gather_addr( column_block_ptr, load1_gather_addr,column_begin);
                        if( num == 1 ) {
                            load1_gather_addr_vec.push_back( load1_gather_addr );
                            load1_gather_index_vec.push_back( inner_i );
                            load1_gather_column_begin_vec.push_back( column_begin );
                            
                            load1_gather_addr_num++;
                        } else {
                            load_gather_index_vec.push_back(inner_i);
                        }
                        int reduce_index_inner_ptr_j_i = 0;
                        for( auto it : load1_gather_addr_vec ) {
                            reduce_index_inner_ptr_j[reduce_index_inner_ptr_j_i] = it;
                            reduce_index_inner_ptr_j_i++;
                        }
                        for( auto it : load_gather_addr_vec ) {
                            reduce_index_inner_ptr_j[reduce_index_inner_ptr_j_i] = it;
                            reduce_index_inner_ptr_j_i++;
                        }
                       
                        reduce_load1_to_gather_num_ptr[j] = load1_gather_addr_num;
                        load1_gather_addr_num_sum += load1_gather_addr_num;
                    
                    }
                 }
                 int64_t * reduce_load1_to_gather_addr_ptr = (int64_t)malloc(sizeof(int64_t)*load1_gather_addr_num_sum);
                 reduce_load1_to_gather_addr_ptr_ptr_[i] = reduce_load1_to_gather_addr_ptr;
                 for( int j = 0 ; j < load1_gather_addr_num_sum ; j++  ) {
                    reduce_load1_to_gather_addr_ptr[j] = load1_gather_index_vec[j];
                 }


             } else {
                 for(int j = 0  ; j < mask_has_data_num ; j++ ) {
                    const int index = index_ptr[j];
                    int load1_gather_addr_num = 0;

                    int * column_block_ptr = m_col_ptr[ (index) * VECTOR  ];
                    int64_t load1_gather_addr;
                    int num = analyze_gather_addr( column_block_ptr, load1_gather_addr);
                    if( num == 1 ) {
                        load1_gather_addr_vec.push_back( load1_gather_addr );
                        load1_gather_addr_num++;
                        load1_gather_index_vec.push_back( index );
                    } else {
                        load_gather_index_vec.push_back( index );
                    } 
                    load1_gather_addr_num_vec.push_back(load1_gather_addr_num);
                 }
                 load1_gather_num_ptr_[ i ] = load1_gather_addr_num;
                 int load_gather_index = 0;
                 for( auto it : load1_gather_index_vec ) {
                    index_ptr[load_gather_index] = it;
                    load_gather_index++;
                 }
                 for( auto it : load_gather_index_vec ) {
                    index_ptr[load_gather_index] = it;
                    load_gather_index++;
                 }                 
                
             }
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
        std::map<int,int> gather_num_map;
        for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             const int mask = mask_ptr_[i];
             int * column_ptr = column_ptr_ptr_[i];
             double * data_ptr = data_ptr_ptr_[i];
             int * index_ptr = index_ptr_ptr_[i];
             int * row_ptr = row_ptr_ptr_[i];

             if( mask == 0x1 ) {
                 int * reduce_index_inner_ptr = reduce_index_inner_ptr_ptr_[i];
                 int * reduce_load1_to_gather_num_ptr = reduce_load1_to_gather_num_ptr_ptr_[i];
                 
                 for(int j = 0  ; j < mask_num ; j++ ) {
                    const int index = index_ptr[j];
                    int reduce_load1_to_gather_num = reduce_load1_to_gather_num_ptr[j];
                    int * reduce_index_inner_ptr_tmp = reduce_index_inner_ptr + j * inner_mask_num * VECTOR;
                    for( int inner_i = 0 ; inner_i < inner_mask_num ; inner_i++ ) {
                        const int inner_i_index = reduce_index_inner_ptr_tmp[inner_i];

                        int * column_block_ptr = &column_ptr[j*VECTOR * inner_mask_num + inner_i * VECTOR ];
                        double * data_block_ptr = &data_ptr[j*VECTOR *inner_mask_num + inner_i * VECTOR ];
                         
                        for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                            column_block_ptr[v_i] = m_col_ptr [(index + inner_i_index) * VECTOR + v_i ];
                            data_block_ptr[v_i]   = m_data_ptr[(index + inner_i_index) * VECTOR + v_i ];
                        }
                    }
                 } 
             } else {
                for(int j = 0  ; j < mask_num ; j++ ) {
                    const int index = index_ptr[j];

                        int * column_block_ptr = &column_ptr[j*VECTOR  ];
                        double * data_block_ptr = &data_ptr[j*VECTOR ];
                         
                        for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                            column_block_ptr[v_i] = m_col_ptr [(index) * VECTOR + v_i ];
                            data_block_ptr[v_i]   = m_data_ptr[(index) * VECTOR + v_i ];
                        }
                    
                }
             }
        }
    }
    public:

    void analyze(int * row_ptr, int * column_ptr, const int row_num ,const int column_num ,double*data_ptr) {
        Analyze( mask_map_,mask_num_map_,mask_num_, row_ptr, column_ptr,row_num, column_num);
        generate_index_and_row_ptr_ptr();

        generate_gather_mask();
        LOG(INFO) << "Before Trans";
        transform( data_ptr, column_ptr);

        LOG(INFO) << "After Trans";

    }

    void generate_func(  ) {
        PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
        page_rank_statement_ptr->make( mask_num_map_ );
        func_ptr_ = page_rank_statement_ptr->get_function();
//        std::cout<<func_ptr_->get_state();
    }
     
    int operator() ( double * y_ptr, double * x_ptr, double * data_ptr, int * column_ptr, int * row_ptr , int row_num ) { 
       (*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, column_ptr_ptr_, data_ptr_ptr_ );
    }
};


#endif
