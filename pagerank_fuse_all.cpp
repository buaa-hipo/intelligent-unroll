#include "pagerank_fuse_all.hpp"
#define LOAD_TO_GATHER_LEVEL 1
    typedef union GatherAddr {
        int64_t data;
        int8_t data_vec[VECTOR];
    } GatherAddr;
    int PageRankFuseAll::analyze_gather_addr(const int * column_block_ptr, int64_t &addr,int & column_begin) {
        GatherAddr gather_addr;
        std::vector<int> column_begin_vec;
        int column_block[VECTOR];
        for( int v_i = 0 ; v_i < VECTOR; v_i++ ) {
            column_block[v_i] = column_block_ptr[v_i];
        }
        int num = 0;
        while(true) {
            int min = INT_MAX ;
            for( int v_i = 0 ; v_i < VECTOR ; v_i++) {
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
                    gather_addr.data_vec[ v_i ] = column_block[v_i] - min;
                    column_block[v_i] = INT_MAX ;
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
    void PageRankFuseAll::generate_gather_mask(const int * m_col_ptr) {
        
        load1_to_gather_num_ptr_ptr_ = (int**)malloc(sizeof(int*)*mask_num_);
        load1_to_gather_addr_ptr_ptr_ = ( int64_t** )malloc(sizeof(int64_t*)*mask_num_);
        load1_to_gather_column_begin_ptr_ptr_ = (int**)malloc(sizeof(int*)*mask_num_);

        reduce_index_mask_ptr_ptr_ = (int**) malloc(sizeof(int*)*mask_num_);
        for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_has_data_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             const int mask = mask_ptr_[i];
             int * index_ptr = index_ptr_ptr_[i];
             std::vector<int64_t> load1_gather_addr_vec;

             std::vector<int> load1_gather_column_begin_vec;

//             std::vector<int> load1_gather_addr_num_vec;
///add mask to for
             if( mask == 0x1 ) {
                 int * load1_to_gather_num_ptr = (int*)malloc(sizeof(int)*mask_has_data_num);
                 load1_to_gather_num_ptr_ptr_[i] = load1_to_gather_num_ptr;

                 int * reduce_index_mask_ptr = (int*)malloc(sizeof(int)* inner_mask_num * mask_has_data_num);
                 reduce_index_mask_ptr_ptr_[i] = reduce_index_mask_ptr;

                 for(int j = 0  ; j < mask_has_data_num ; j++ ) {
                    const int index = index_ptr[j];
                    int * reduce_index_mask_ptr_j = reduce_index_mask_ptr + j * inner_mask_num;
                    const int before_size = load1_gather_addr_vec.size();

                    for( int inner_i = 0 ; inner_i < inner_mask_num ; inner_i++ ) {

                        const int * column_block_ptr = &m_col_ptr[ (index + inner_i) * VECTOR  ];
                        int64_t load1_gather_addr;
                        int  column_begin;
                        int num = analyze_gather_addr( column_block_ptr, load1_gather_addr,column_begin);
                        if( num == 1 ) {
                            load1_gather_addr_vec.push_back( load1_gather_addr );
                            load1_gather_column_begin_vec.push_back( column_begin );
                            reduce_index_mask_ptr_j[inner_i] = 1;                        
                        } else {
                            reduce_index_mask_ptr_j[inner_i] = VECTOR;
                        }
                    }

                    load1_to_gather_num_ptr[j] = load1_gather_addr_vec.size() - before_size;
                 }


             } else {
                 std::vector<int> load1_gather_index_vec;

                 std::vector<int> load_gather_index_vec;

                 for(int j = 0  ; j < mask_has_data_num ; j++ ) {
                    const int index = index_ptr[j];

                    const int * column_block_ptr = &m_col_ptr[ (index) * VECTOR  ];
                    int64_t load1_gather_addr;
                    int column_begin;
                    int num = analyze_gather_addr( column_block_ptr, load1_gather_addr,column_begin);
                    if( num == 1 ) {
                        load1_gather_addr_vec.push_back( load1_gather_addr );
                        load1_gather_index_vec.push_back( index );
                        load1_gather_column_begin_vec.push_back(column_begin);
                    } else {
                        load_gather_index_vec.push_back( index );
                    } 
                 }
                 int * load1_to_gather_num_ptr = (int*)malloc(sizeof(int));
                 load1_to_gather_num_ptr_ptr_[i] = load1_to_gather_num_ptr;

                 load1_to_gather_num_ptr[ 0 ] = load1_gather_index_vec.size();
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
            /////////////////////
             int64_t * load1_to_gather_addr_ptr = (int64_t*)malloc(sizeof(int64_t)* load1_gather_addr_vec.size());
             load1_to_gather_addr_ptr_ptr_[i] = load1_to_gather_addr_ptr;
             int it_i = 0;
             for( auto it : load1_gather_addr_vec ) {
                load1_to_gather_addr_ptr[it_i] = it;
                it_i++;
             }


             //////////////////
             int column_begin_size = load1_gather_column_begin_vec.size();
             int * load1_gather_column_begin_ptr = (int*)malloc(sizeof(int)*column_begin_size);
             load1_to_gather_column_begin_ptr_ptr_[i] = load1_gather_column_begin_ptr;
             LOG(INFO) << std::hex <<mask << " "<< std::dec << mask_has_data_num <<" " << column_begin_size;
             for( it_i = 0 ; it_i < column_begin_size ; it_i++ ) {
                load1_gather_column_begin_ptr[it_i] = load1_gather_column_begin_vec[it_i];
             }
             
        }
    }



    void PageRankFuseAll::generate_index_and_row_ptr_ptr( ) {
        index_ptr_ptr_ = (int**)malloc(sizeof( int*) * mask_num_);
        num_ptr_ = (int*) malloc(sizeof(int)*mask_num_);
        mask_ptr_ = (int*) malloc(sizeof(int)*mask_num_);
        inner_num_ptr_ = (int*)malloc(sizeof(int)*mask_num_);
        int i = 0 ;
        for( const auto & it : mask_num_map_ ) {
             index_ptr_ptr_[i] = (int*)malloc(sizeof(int)*it.second );
             inner_num_ptr_[i] = it.first.num_;
             num_ptr_[i] = it.second;
             mask_ptr_[i] = it.first.mask_;
             i++;
        }
        i = 0 ;
        for( const auto & it : mask_map_ ) {

            const std::vector<int> & index_vec = it.second.second;

            int jj = 0 ;
            for( auto it : index_vec) {
                index_ptr_ptr_[i][jj] = it;
                jj++;
            }
            i++;
        }
    }


void PageRankFuseAll::transform(double * m_data_ptr, int * m_col_ptr) {
        column_ptr_ptr_ = ( int ** )malloc(sizeof(int*)*mask_num_);
        data_ptr_ptr_ = (double**)malloc(sizeof(double*)*mask_num_);
        row_ptr_ptr_ = ( int** )malloc(sizeof(int*) * mask_num_);

        ////malloc
         for( int i = 0 ; i < mask_num_ ; i++ ) {
             const int mask_num = num_ptr_[i];
             const int inner_mask_num = inner_num_ptr_[i];
             int * column_ptr = (int*)_mm_malloc(sizeof(int)* mask_num * inner_mask_num*VECTOR,ANONYMOUSLIB_X86_CACHELINE);
             double * data_ptr = (double*)_mm_malloc(sizeof(double)* mask_num * inner_mask_num*VECTOR,ANONYMOUSLIB_X86_CACHELINE);

             int * row_ptr = (int*)_mm_malloc(sizeof(int)*mask_num,ANONYMOUSLIB_X86_CACHELINE);
             if( column_ptr == NULL || data_ptr == NULL || row_ptr == NULL ) {
                LOG(FATAL) << "malloc fatal";
             } else {
                row_ptr_ptr_[i] = row_ptr;
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
             
             int * row_ptr = row_ptr_ptr_[i];
             double * data_ptr = data_ptr_ptr_[i];
             int * index_ptr = index_ptr_ptr_[i];

             int * load1_to_gather_column_begin_ptr = load1_to_gather_column_begin_ptr_ptr_[i];

             int64_t *  load1_to_gather_addr_ptr = load1_to_gather_addr_ptr_ptr_[i] ;
             if( mask == 0x1 ) {
                 int * reduce_load1_to_gather_num_ptr = load1_to_gather_num_ptr_ptr_[i];

                 int * reduce_index_mask_ptr = reduce_index_mask_ptr_ptr_[i] ;
                 int * reduce_load1_to_gather_column_begin_ptr = load1_to_gather_column_begin_ptr;
                LOG(INFO) << mask_num;
                 for(int j = 0  ; j < mask_num ; j++ ) {
                    int * column2_ptr = &column_ptr[ j * inner_mask_num * VECTOR ];

                    double * data2_ptr = &data_ptr[ j * inner_mask_num * VECTOR ];
                    const int index = index_ptr[j];
                    row_ptr[ j ] = m_row_ptr_[index * VECTOR];
                    int reduce_load1_to_gather_num = reduce_load1_to_gather_num_ptr[j];
                    const int * reduce_index_mask_ptr_tmp = reduce_index_mask_ptr + j * inner_mask_num ;
                     
                    int index1 = 0;
                    for( int inner_i = 0,  index_vec = 0 ; inner_i < inner_mask_num ; inner_i++ ) {
                        const int inner_i_mask = reduce_index_mask_ptr_tmp[inner_i];

                        if( inner_i_mask == 1 ) {

                            double * data_block_ptr = &data2_ptr[ (index1 ) * VECTOR ];
                            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                                data_block_ptr[v_i] = m_data_ptr[(index + inner_i) * VECTOR + v_i ];
                            }
                            column2_ptr[ index1 ] = reduce_load1_to_gather_column_begin_ptr[index1];
                            #ifdef DEBUG
                            int8_t * column_off_tmp = (int8_t*) (&load1_to_gather_addr_ptr[ index1 ]); 
                            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                                CHECK(column2_ptr[index1] + column_off_tmp[v_i] == m_col_ptr[(index + inner_i) * VECTOR + v_i ]  ) << "they should be equal";
                            }
                            #endif
                            index1++;
                        } else {

                            double * data_block_ptr = &data2_ptr[ (index_vec+reduce_load1_to_gather_num) * VECTOR ];

                            int * column_block_ptr = &column2_ptr[(index_vec+reduce_load1_to_gather_num) * VECTOR ];
                            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                                data_block_ptr[v_i] = m_data_ptr[ (index + inner_i) * VECTOR + v_i ];
                                column_block_ptr[v_i] = m_col_ptr[ (index + inner_i) * VECTOR + v_i ]; 
                            }

                            index_vec++;
                        }
                    }
                    CHECK(index1 == reduce_load1_to_gather_num) << "they should be equal";
                    reduce_load1_to_gather_column_begin_ptr += reduce_load1_to_gather_num; 
                 }
                 LOG(INFO) << "Before Free";
                 free( reduce_index_mask_ptr );
                 LOG(INFO) << "After Free"; 
             } else {

                const int * load1_to_gather_num_ptr = load1_to_gather_num_ptr_ptr_[i];
                const int load1_gather_num = load1_to_gather_num_ptr[0];
                for( int j = 0 ; j < load1_gather_num ; j++ ) {
                    column_ptr[j] = load1_to_gather_column_begin_ptr[ j ]; 

                    const int index = index_ptr[j];
                    row_ptr[ j ] = m_row_ptr_[index * VECTOR];
                    #ifdef DEBUG

                    int8_t * column_off_tmp = (int8_t*) (&load1_to_gather_addr_ptr[ j ]); 
                    for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                        CHECK(column_ptr[j] + column_off_tmp[v_i] == m_col_ptr[(index) * VECTOR + v_i ]  ) << "they should be equal";
                    }
                    #endif

                }
                for(int j = load1_gather_num  ; j < mask_num ; j++ ) {
                    const int index = index_ptr[j];

                    row_ptr[ j ] = m_row_ptr_[index * VECTOR];
                    int * column_block_ptr = &column_ptr[j*VECTOR  ];
                         
                    for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                        column_block_ptr[v_i] = m_col_ptr [(index) * VECTOR + v_i ];
                    }
                    
                }

                for(int j = 0  ; j < mask_num ; j++ ) {
                    const int index = index_ptr[j];

                    double * data_block_ptr = &data_ptr[j*VECTOR ];
                         
                    for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                        data_block_ptr[v_i]   = m_data_ptr[(index) * VECTOR + v_i ];
                    }
                    
                }

             }

             free( load1_to_gather_column_begin_ptr );

        }
    }
    void PageRankFuseAll::generate_func(  ) {
        PageRankStateMent * page_rank_statement_ptr = new PageRankStateMent( );
        page_rank_statement_ptr->make( mask_num_map_);
        func_ptr_ = page_rank_statement_ptr->get_function();
        std::cout<<func_ptr_->get_state();
    }
    void PageRankFuseAll::analyze(int * row_ptr, int * column_ptr, const int row_num ,const int column_num ,double*data_ptr) {
        m_row_ptr_ = (int*)malloc(sizeof(int) * row_ptr[row_num]);
        for( int i = 0 ; i < row_num; i++ ) {
            int row_begin = row_ptr[i];
            int row_end = row_ptr[i+1];
            for( int j = row_begin ; j < row_end ; j++ ) {
                m_row_ptr_[j] = i;
            }
        }
        Analyze( mask_map_,mask_num_map_,mask_num_, row_ptr, column_ptr,row_num, column_num);
        
        generate_index_and_row_ptr_ptr();

        generate_gather_mask( column_ptr );
        LOG(INFO) << "Before Trans";
        transform( data_ptr, column_ptr);

        LOG(INFO) << "After Trans";

    }

