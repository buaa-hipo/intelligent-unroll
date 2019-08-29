#include "tools_set.hpp"
#include "hash_feature_table.hpp"
    template<typename G_R>
    void generate_info(G_R * info_ptr, int * addr, int len, G_R(*func_ptr)(int*) ) {
        int vec_num = len ;
        info_ptr = (G_R*)malloc(sizeof(G_R) * vec_num );
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = func_ptr( addr + i * VECTOR );
        }
    }

ScatterInfo * generate_disorder_info_elem( int * elem_addr ) {
    ScatterInfo * scatter_info = new ScatterInfo();
        DisorderAddr disorder_addr;
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
                    disorder_addr.data_vec[ v_i ] = column_block[v_i] - min;
                    column_block[v_i] = INT_MAX ;
                }
            }
        }
        int level;
        if( num <= LOAD_TO_GATHER_LEVEL ) {
            addr = gather_addr.data;

            scatter_info->data_index[0] = column_begin_vec[0];
            level = num;
        } else {
            level = -1;
        }
    scatter_info->mask_ = level;
    scatter_info->disorder_addr_ = scatter_addr;
    return scatter_info;
}
 OrderType get_order_type(int * index_ptr) {
    bool is_continue = true;
    int inc_num = 0;
    int dec_num = 0;
    int equal_num = 0;
    int inc_continue_num = 0;
    int dec_continue_num = 0;

    for( int i = 1 ; i < VECTOR ; i++ ) {
        int before_index = index_ptr[i-1];
        int after_index = index_ptr[i];
        if( before_index < after_index) {
            inc_num++;
            inc_continue_num = before_index + 1 == after_index ? inc_continue_num + 1 : inc_continue_num;
        } else if( before_index > after_index ) {
            dec_num++;

            dec_continue_num = before_index - 1 == after_index ? dec_continue_num + 1 : dec_continue_num;
        } else {
            equal_num++;
        }
    }
    if( equal_num == VECTOR ) {
        return Equel;
    } else if( inc_continue_num + equal_num == VECTOR ) {
        return IncContinue;
    } else if(dec_continue_num + equal_num == VECTOR) {
        return DecContinue;
    } else if( inc_num + equal_num == VECTOR ) {
        return Inc;
    } else if( dec_num + equal_num == VECTOR ) {
        return Dec;
    } else {
        return DisOrder;
    }
}
int get_mask(int * index_ptr,OrderType order_type) {

    int mask = 0x1;
    if( order_type != DisOrder ) {
        int cur_index = index_ptr[0];
        for( int i = 1 ; i < VECTOR ; i++ ) {
            if(cur_index != index_ptr[i]) {
                mask |= (1<<i);
                cur_index = index_ptr[i];
            }
        }
        mask |= (order_type << (VECTOR) );
    } else {
        LOG(FATAL) << "Unsupported";
    }
    return mask;
}

ScatterInfo generate_scatter_info_elem(int * index_ptr) {
    ScatterInfo scatter_info;

    OrderType order_type = get_order_type( index_ptr );
    if(order_type == Equel) {
        scatter_info.mask_ = 0x1;
        scatter_info.is_continue_ = true;
        scatter_info.write_index[0] = index_ptr[0];
    } else if( order_type == IncContinue ) {
        scatter_info.mask_ = get_mask( index_ptr, order_type );
        scatter_info.order_type_ = order_type;
        scatter_info.write_index[0] = index_ptr[0];
    } else if( order_type == DisOrder ) {
        ScatterInfo * scatter_info_ptr = generate_disorder_info_elem( index_ptr );
        scatter_info = *scatter_info_ptr;
    } else {
        LOG(FATAL) << "Unsupported"; 
    }
    return scatter_info;
}
GatherInfo generate_gather_info_elem(int * index_ptr) {
    return generate_scatter_info_elem(index_ptr);
}
ReductionInfo generate_reduction_info_elem(int * index_ptr,const ScatterInfo &scatter_info ) {
    ReductionInfo reduction_info;
    if(scatter_info.order_type_ == IncContinue || scatter_info.order_type_==Equel ) {
         reduction_info.order_type = scatter_info.order_type_; 
         reduction_info.mask_ = scatter_info.mask_;
    } else {
        LOG(FATAL) << "Unsupported Now";
    }
    return reduction_info;
}

void ToolsSet::generate_info( ScatterInfo *info_ptr ,int *addr,int len) {
    generate_info<ScatterInfo>( info_ptr, addr,len,generate_scatter_info_elem );
}
void ToolsSet::generate_info( ReductionInfo *info_ptr ,int *addr,int len, ScatterInfo * scatter_info_ptr ) {
        int vec_num = len ;
        info_ptr = (ReductionInfo*)malloc(sizeof(ReductionInfo) * vec_num );
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = generate_reduction_info_elem( addr + i * VECTOR );
        }
}

void ToolsSet::generate_info( GatherInfo *info_ptr ,int *addr,int len) {
    generate_info<GatherInfo>( info_ptr, addr,len,generate_gather_info_elem );
}
size_t *  get_feature_hash( Info** info_vec, const int table_row_num,const int table_column_num ) {
    size_t * key_ptr = (size_t*)malloc(sizeof(size_t) * table_column_num);
    for( int i = 0 ; i < table_column_num ; i++ ) {
        size_t key = 0;
        for( int j = 0 ; j < table_row_num ; j++ ) {
            key = HashFeatureTable::HashCombine( key , info_vec[j][i].get_mask() );
        }
        key_ptr[i] = key;
    }
    return key_ptr;
} 


void combin_same_feature_together( std::unordered_map<size_t, std::vector<int>> & same_feature_map , size_t * hash_info_ptr , const int table_column_num ) {
    
    for( int i = 0 ; i < table_column_num ; i++ ) {
         same_feature_map[ hash_info_ptr[ i ] ].push_back( i );
    }
}
void combin_same_write_pattern_together_elem( std::vector<std::pair<int,int>> &same_write_range , std::vector<int> & index_vec,ScatterInfo * info_ptr ) {
    const int index_vec_size = index_vec.size();
    if(index_vec.size() == 0 ) {
        return;
    }
    const ScatterInfo & scatter_info = info[index_vec[0]];
    int old_write_local = scatter_info.write_index[0];  
    int old_write_local_index = 0;
    int j = 0;
    for( int i = 1, j = 0; i < index_vec_size ; i++ ) {
        const ScatterInfo & scatter_info = info[index_vec[i]];
        int write_local = scatter_info.write_index[i];
        if(write_local == old_write_local) {
            
        } else {
            if( old_write_local_index + 1 != i ) {
                same_write_range.push_back( std::pair<int>(old_write_local_index, i  ) ); 
            } else {
                index_vec[ j ] = old_write_local;
                j++;
            }
            old_write_local_index = i;
            old_write_local = write_local;;
        }
    }
    if( scatter_info.write_index[ index_vec_size - 1 ] == old_write_local ) {
        if( old_write_local_index + 1 != index_vec_size ) {
            same_write_range.push_back( std::pair<int>(old_write_local_index,  index_vec_size ) ); 
        } else {
            index_vec[ j ] = old_write_local;
            j++;
        }   
    } else {
        LOG(FATAL) << "This Condition should never happend";
    }
    
    if( j != index_vec_size) {
        index_vec.erase( index_vec.begin() + j , index_vec.begin + index_vec_size); 
    } 
}

void combin_same_write_pattern_together( std::unordered_map<size_t,std::vector<int>> & same_feature_map, std::unordered_map<size_t,std::vector<std::pair<int,int>>> & same_write_pattern_map, ScatterInfo * info ) {
    for( auto &it = same_feature_map.begin() ; it != same_feature_map.end(); ) {
          const size_t & feature_hash = it.first;
          combin_same_write_pattern_together_elem( same_write_pattern_map[feature_hash], it.second  , info);
          if( same_write_pattern_map[feature_hash].size() == 0 ) {
                same_write_pattern_map.erase(feature_hash);  
          }
          auto & origin = it;
          it++;
          if( it.second.size() == 0 ) {
                same_feature_map.erase( origin );
          }
    }
}




    //////////////////
    void generate_information( 
        const std::set<std::string> & scatter_set,
        const std::set<std::string> & reduction_set,
        const std::set<std::string> & gather_set,
        const std::map<std::string, void*> & name2ptr_map,
        const int table_column_num,
        ////output
        std::map<std::string,GatherInfo*> &gather_map,
        std::map<std::string,ScatterInfo*> &scatter_map,
        std::map<std::string,ReductionInfo*> &reduction_map,
        std::unordered_map<size_t,std::vector<int>> &same_feature_map_,
        std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map
        ) {
        //////Init feature table
        const int gather_num = gather_set.size();
        const int scatter_num = scatter_set.size();
        const int reduction_num = reduction_set_.size(); 
        const int args_num = gather_num + scatter_num + reduction_num;

        Info ** info_table = (Info**) malloc(sizeof(Info*)*args_num);
        int table_row_begin = 0;
        int reduction_i = 0;
        int scatter_i = 0;
        for( auto scatter_index : scatter_set ) {
            ScatterInfo * scatter_info_tmp = new ScatterInfo[ table_column_num_ ];
            table_row_i = table_row_begin + scatter_i; 
            generate_info( scatter_info_tmp , (int*)name2ptr_map[ table_row_i ] , table_column_num );
            
            info_table[ table_row_i ] = scatter_info_tmp;
            scatter_map[ scatter_index ] = scatter_info_tmp;
            ////////////reduction
            if( reduction_set.find(scatter_index) != reduction_set.end()  ) {

                ReductionInfo * reduction_info_tmp = new ReductionInfo[ table_column_num_ ];
                generate_info( reduction_info_tmp, (int*)args_ptr[table_row_i], table_column_num  );
                info_table[ table_row_i + 1 ] = reduction_info_tmp;
                reduction_map[scatter_index] = reduction_info_tmp; 
                gather_map[ scatter_index ] = scatter_info_tmp; 
                scatter_i++;
            }
            scatter_i++;
        }
        table_row_begin += scatter_num + reduction_num;
        int gather_i = 0;
        for( auto gather_index : gather_set_ ) {
            GatherInfo * gather_info_tmp = new GatherInfo[ table_column_num_ ];
            table_row_i = table_row_begin + gather_i; 
            generate_info( gather_info_tmp , args_ptr[ table_row_i ] , table_column_num );
            info_table[ table_row_i ] = gather_info_tmp;
            gather_map[ gather_index ] = gather_info_tmp;
            gather_i++;
        }
        /////////////////
        size_t * hash_table = get_feature_hash( info_table_, args_num, table_column_num );
        /////////////////combin same feature
        combin_same_feature_together( same_feature_map, hash_table,table_column_num);
        ////combine same write location pattern
        ///assume that the number of scatter or store operation is only one
        /////
        CHECK(scatter_num == 1 || scatter_num == 0) << "scatter_num should be 1 or 0"
        if(scatter_num == 1)
            combin_same_write_pattern_together( same_feature_map, same_feature_range_map, dynamic_cast<ScatterInfo>(info_table_[0] ));

    }

