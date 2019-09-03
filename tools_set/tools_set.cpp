#include "tools_set.hpp"
#include "hash_feature_table.hpp"
#include "log.h"
std::ostream & operator<<( std::ostream &stream , const DisorderAddr & in ) {
    
    for( int i = 0 ; i < VECTOR ; i++ )
        stream << (int)in.data_vec[i] << " ";
    stream << std::endl;
    return stream;
}

std::ostream & operator<<( std::ostream &stream , const GatherInfo &scatter_info ) {
    stream << scatter_info.order_type_;
    if(scatter_info.order_type_ == DisOrder) {
        stream << scatter_info.disorder_addr_;
        stream <<( scatter_info.get_mask() & VEC_MASK)<<"\n";
        for( int i = 0 ; i < (scatter_info.get_mask() & VEC_MASK) ; i++ ) {
            stream << scatter_info.data_index_[i];
        }
    } else {
        stream <<" " << scatter_info.data_index_[0] << "\n";
    }
    return stream;
}


    template<typename G_R>
    void generate_info(G_R * info_ptr, const int * addr, const int vec_num, G_R(*func_ptr)(const int*) ) {
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = func_ptr( addr + i * VECTOR );
        }
    }

ScatterInfo generate_disorder_info_elem(const int * elem_addr ) {
    ScatterInfo scatter_info;
        DisorderAddr disorder_addr;
        std::vector<int> column_begin_vec;
        int column_block[VECTOR];
        for( int v_i = 0 ; v_i < VECTOR; v_i++ ) {
            column_block[v_i] = elem_addr[v_i];
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
#define LOAD_TO_GATHER_LEVEL 1
        if( num <= LOAD_TO_GATHER_LEVEL ) {

            scatter_info.data_index_[0] = column_begin_vec[0];
            level = num;
            scatter_info.disorder_addr_ = disorder_addr;
        } else {
            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) 
                scatter_info.data_index_[v_i] = elem_addr[v_i];
            level = VECTOR;
        }
    scatter_info.mask_ = level;
    return scatter_info;
}
 OrderType get_order_type(const int * index_ptr) {
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
    OrderType ot;
    if( equal_num == VECTOR - 1) {
        ot =  OrderEquel;
    } else if( inc_continue_num + equal_num == VECTOR - 1 ) {
        ot = IncContinue;
    } else if(dec_continue_num + equal_num == VECTOR - 1) {
        ot = DecContinue;
    } else if( inc_num + equal_num == VECTOR - 1) {
        ot = Inc;
    } else if( dec_num + equal_num == VECTOR - 1) {
        ot =  Dec;
    } else {
        ot = DisOrder;
    }
    return ot;
}
int get_mask(const int * index_ptr,OrderType order_type) {

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

ScatterInfo generate_scatter_info_elem(const int * index_ptr) {
    ScatterInfo scatter_info;

    OrderType order_type = get_order_type( index_ptr );
    if(order_type == OrderEquel) {
        scatter_info.mask_ = 0x1;
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == IncContinue ) {
        scatter_info.mask_ = get_mask( index_ptr, order_type );
        scatter_info.order_type_ = order_type;
        scatter_info.data_index_[0] = index_ptr[0];
    } else if( order_type == DisOrder ) {
        scatter_info = generate_disorder_info_elem( index_ptr );
    } else {
        LOG(FATAL) << "Unsupported"; 
    }
    return scatter_info;
}
GatherInfo generate_gather_info_elem(const int * index_ptr) {
    return generate_scatter_info_elem(index_ptr);
}
ReductionInfo generate_reduction_info_elem(const int * index_ptr,const ScatterInfo &scatter_info ) {
    ReductionInfo reduction_info;
    if(scatter_info.order_type_ == IncContinue || scatter_info.order_type_== OrderEquel ) {
         reduction_info.order_type_ = scatter_info.order_type_; 
         reduction_info.mask_ = scatter_info.mask_;
    } else {
        LOG(FATAL) << "Unsupported Now" << scatter_info.order_type_;
    }
    return reduction_info;
}

void generate_info( ScatterInfo *info_ptr ,const int *addr,int len) {
    generate_info<ScatterInfo>( info_ptr, addr,len,generate_scatter_info_elem );
}
void generate_info( ReductionInfo *info_ptr ,const int *addr, const int vec_num , ScatterInfo * scatter_info_ptr ) {
        for( int i = 0 ; i < vec_num ; i++ ) {
            info_ptr[i] = generate_reduction_info_elem( addr + i * VECTOR, scatter_info_ptr[i] );
        }
}

void generate_info( GatherInfo * info_ptr ,const int *addr,int len) {
    generate_info<GatherInfo>( info_ptr, addr,len,generate_gather_info_elem );
}
size_t *  get_feature_hash( const std::map<std::string,GatherInfo*> &gather_map,
                            const std::map<std::string,ScatterInfo*> & scatter_map,
                            const std::map<std::string,ReductionInfo*>&reduction_map,
                            const int table_column_num ) {
//    size_t * key_ptr = (size_t*)malloc(sizeof(size_t) * table_column_num);
    size_t * key_ptr = new size_t[ table_column_num ]; 
    for( int i = 0 ; i < table_column_num ; i++ ) {
        size_t key = 0;
        key_ptr[i] = key;
    }
    for( auto it : gather_map ) {
         GatherInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }
    for( auto it : scatter_map ) {
         ScatterInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }
    for( auto it : reduction_map ) {
         ReductionInfo * info_ptr = it.second;

         for( int i = 0 ; i < table_column_num ; i++ ) 
            key_ptr[i] = HashFeatureTable::HashCombine( key_ptr[i] , info_ptr[i].get_mask() );
    }

    return key_ptr;
} 


void combin_same_feature_together( std::unordered_map<size_t, std::vector<int>> & same_feature_map , size_t * hash_info_ptr , const int table_column_num ) {
    
    for( int i = 0 ; i < table_column_num ; i++ ) {
/*         if( same_feature_map.find( hash_info_ptr[i] ) == same_feature_map.end() ) {
               
             same_feature_map[hash_info_ptr[i]] = std::vector<int>();
         }
         */
         same_feature_map[ hash_info_ptr[ i ] ].push_back( i );
    }
}
void combin_same_write_pattern_together_elem( std::vector<std::pair<int,int>> &same_write_range , std::vector<int> & index_vec,const ScatterInfo * info_ptr ) {
    const int index_vec_size = index_vec.size();
    if(index_vec.size() == 0 ) {
        return;
    }
    const ScatterInfo & scatter_info = info_ptr[index_vec[0]];
    if( scatter_info.order_type_ == OrderEquel ) {
        int old_write_local = scatter_info.data_index_[0];  
        int old_write_local_index = 0;
        int j = 0;
        int write_local = old_write_local;
        for( int i = 1; i < index_vec_size ; i++ ) {
            const ScatterInfo & scatter_info_tmp = info_ptr[index_vec[i]];
            write_local = scatter_info_tmp.data_index_[0];
            if(write_local == old_write_local) {
            
            } else {
                if( old_write_local_index + 1 < i ) {
                    same_write_range.push_back( std::pair<int,int>(old_write_local_index, i  ) ); 
                } else {
                    index_vec[ j ] = old_write_local;

                    j++;
                }
                old_write_local_index = i;
                old_write_local = write_local;;
            }
        }

        if( write_local == old_write_local ) {
            if( old_write_local_index + 1 != index_vec_size ) {
                same_write_range.push_back( std::pair<int,int>(old_write_local_index,  index_vec_size ) ); 
            } else {
                index_vec[ j ] = old_write_local;
                j++;
            }   
        } else {
            LOG(FATAL) << "This condition never happend";
        }
        if( j != index_vec_size) {
            index_vec.erase( index_vec.begin() + j , index_vec.begin() + index_vec_size); 
        } 

    }
}

void combin_same_write_pattern_together( std::unordered_map<size_t,std::vector<int>> & same_feature_map, std::unordered_map<size_t,std::vector<std::pair<int,int>>> & same_write_pattern_map, const ScatterInfo * info ) {
    std::vector<size_t> erase_vec;
    for(  auto & it : same_feature_map ) {
          const size_t & feature_hash = it.first;
          combin_same_write_pattern_together_elem( same_write_pattern_map[feature_hash], it.second  , info);

          if( same_write_pattern_map[feature_hash].size() == 0 ) {
                same_write_pattern_map.erase(feature_hash);  
          }
          if( it.second.size() == 0 ) {
                erase_vec.push_back(it.first);
          }
    }
    for( auto it : erase_vec )
        same_feature_map.erase( it );
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
        std::unordered_map<size_t,std::vector<int>> &same_feature_map,
        std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map
        ) {
        //////Init feature table
        LOG(INFO) << table_column_num;
        std::string output_name;
        for( const auto & scatter_index : scatter_set ) {
            output_name = scatter_index;
            ScatterInfo * scatter_info_tmp = new ScatterInfo[table_column_num] ;

            auto name2ptr_map_it = name2ptr_map.find(scatter_index);

            generate_info( scatter_info_tmp , (int*)name2ptr_map_it->second , table_column_num );
            scatter_map[ scatter_index ] = scatter_info_tmp;
            ////////////reduction
            if( reduction_set.find(scatter_index) != reduction_set.end()  ) {

                ReductionInfo * reduction_info_tmp = new ReductionInfo[ table_column_num ];

                generate_info( reduction_info_tmp, (int*)name2ptr_map_it->second, table_column_num ,scatter_info_tmp );

                reduction_map[scatter_index] = reduction_info_tmp; 
                gather_map[ scatter_index ] = scatter_info_tmp; 
            }
        }
        for( const auto & gather_index : gather_set ) {

            GatherInfo * gather_info_tmp = new GatherInfo[table_column_num];
            auto name2ptr_map_it = name2ptr_map.find(gather_index);
            generate_info( gather_info_tmp , (int*)name2ptr_map_it->second , table_column_num );

            gather_map[ gather_index ] = gather_info_tmp;
        }
        LOG(INFO) << table_column_num;
        /////////////////
        size_t * hash_table = get_feature_hash(  gather_map,scatter_map,reduction_map,table_column_num );
        /////////////////combin same feature
        combin_same_feature_together( same_feature_map, hash_table,table_column_num);
        ////combine same write location pattern
        ///assume that the number of scatter or store operation is only one
        /////

        const int scatter_num = scatter_set.size();
        CHECK(scatter_num == 1 || scatter_num == 0) << "scatter_num should be 1 or 0";
        if(scatter_num == 1) {

            ScatterInfo * scatter_info_tmp = scatter_map[ output_name ];
            CHECK(scatter_info_tmp!=NULL);
            combin_same_write_pattern_together( same_feature_map, same_feature_range_map, scatter_info_tmp);
        }

    }

    std::ostream &  operator << (std::ostream & stream , const OrderType &order_type) {
        std::string order_type_str [] = {
            "DisOrder",
            "Inc",
            "Dec",
            "IncContinue",
            "DecContinue",
            "OrderEquel"
        }; 
        stream << order_type_str[ order_type ];
        return stream;
    }
