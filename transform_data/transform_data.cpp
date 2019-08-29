#include "transform_data.hpp"
class TransformData {
    //data need to be transform
    const std::set<std::string> & transform_data_set_;
    const std::map<std::string, VarType > & name_type_map_;
    const std::map<std::string,void*> & name_ptr_map_;
    const std::set<std::string > & gather_set_;
    const std::set<std::string> & scatter_set_;
    const std::set<std::string> & reduction_set_;

    ////output
    std::map<std::string,int*> & gather_name_new_ptr_map_;
    std::map<std::string,int*> & reduction_name_new_ptr_map_;
    std::map<std::string,int*> & scatter_name_new_ptr_map_;

    std::map<std::string,void*> & name_new_ptr_map_;
    //index
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map_;
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map_;
    ///Construct function
    TransformData(
    const std::set<std::string> & transform_data_set,
    const std::map<std::string, VarType > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    ///output
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map
            ) : 
        transform_data_set_(transform_data_set),
        name_type_map_(name_type_map),
        name_ptr_map_(name_ptr_map),
        gather_set_( gather_set ),
        scatter_set_(scatter_set),
        reduction_set_(reduction_set),
        gather_name_new_ptr_map_(gather_name_new_ptr_map),
        reduction_name_new_ptr_map_(reduction_name_new_ptr_map),
        scatter_name_new_ptr_map_(scatter_name_new_ptr_map),
        name_new_ptr_map_(name_new_ptr_map),
        same_feature_map_(same_feature_map),
        same_feature_range_map_(same_feature_range_map)
    {
    }
    ///
    int vec_num_;
    template<typename T>
    int rearrange_elem(const int index, T * data_ptr,int i,int * new_data_ptr) {
        int inner_i ;
        for( inner_i = 0 ; inner_i < VECTOR ; inner_i++ ) {
            new_data_ptr[ i * VECTOR + inner_i ] = data_ptr[index * VECTOR + inner_i];
        }
        i++;
        return i; 
    }

    template<>
    int rearrange_elem(const int index, GatherInfo * data_ptr,int i,int * new_data_ptr) {
                const GatherInfo & gather_info_tmp = data_ptr[ index ];
                if( gather_info_tmp.order_type_ == DisOrder ) {
                if( gather_info_tmp.get_mask() != VECTOR ) {
                for( int disorder_i = 0 ; disorder_i < (VECTOR >> 2) ; disorder_i++) {
                    new_data_ptr[ i  ] = gather_info_tmp.disorder_addr_.int_data_vec_[disorder_i];
                    i++;
                }
                }
                for( int mask_i = 0 ; mask_i < gather_info_tmp.get_mask() ; mask_i++) {
                
                    new_data_ptr[ i  ] = gather_info_tmp.data_index_[mask_i];
                }
                }
            return i;
    }
    template<>
    int rearrange_elem(const int index, ReductionInfo * data_ptr,int i,int * new_data_ptr) {
                const ReductionInfo & reduction_info_tmp = data_ptr[ index ];
                if( reduction_info_tmp.order_type_ == DisOrder ) {
                    int mask = reduction_info_tmp.get_mask();
                    for( int mask_i = 0 ; mask_i < mask ; mask_i++ ) {
                        for( int disorder_i = 0 ; disorder_i < (VECTOR >> 2) ; disorder_i++) {
                            new_data_ptr[ i  ] = gather_info_tmp.reduction_addr_[mask_i].addr_.int_data_vec_[disorder_i];
                            i++;
                        }
                    }
                } 
            return i;
    }
    template<typename T>
    T * malloc_new_data( T * data_ptr ) {
        T * new_data_ptr = ( T* )malloc(sizeof( T ) * vec_num_ * VECTOR);
        return new_data_ptr;
    }
    template<>
    int * malloc_new_data( ReductionInfo * data_ptr ) {
        int disorder_gather_num = 0;
        int need_data_num = 0;
        for( int i = 0 ; i < vec_num_ ; i++ ) {
            if( data_ptr[i].order_type_ == DisOrder ) {
                disorder_gather_num++;
                const int mask = data_ptr[i].get_mask();
                if( mask != VECTOR )
                    need_data_num += sizeof( DisorderAddr ) / sizeof(int) ;            
            } 
        }
        int * new_data_ptr = NULL;
        if( disorder_gather_num > 0 ) 
            new_data_ptr = ( int* )malloc(sizeof( int ) *  need_data_num );
        return new_data_ptr;
    }

    template<>
    int * malloc_new_data( GatherInfo * data_ptr ) {
        int disorder_gather_num = 0;
        int need_data_num = 0;
        for( int i = 0 ; i < vec_num_ ; i++ ) {
            if( data_ptr[i].order_type_ == DisOrder ) {
                disorder_gather_num++;
                const int mask = data_ptr[i].get_mask();
                need_data_num += mask;
                if( mask != VECTOR )
                    need_data_num += sizeof( DisorderAddr ) / sizeof(int) ;            
            }
        }
        int * new_data_ptr = NULL;
        if( disorder_gather_num > 0 ) 
            new_data_ptr = ( int* )malloc(sizeof( int ) *  need_data_num );
        return new_data_ptr;
    }


    template<typename R,typename InfoT>
    R * rearrange( InfoT * data_ptr ) {
       R * new_data_ptr = malloc_new_data(data_ptr);
       if(new_data_ptr != NULL) { 
       int i = 0;
        for( auto & same_feature_it : same_feature_map_ ) {
            std::vector<int> & vec_tmp = same_feature_it.second;
            for( auto index : vec_tmp ) {
                
                i = rearrange_elem( index, data_ptr, i, new_data_ptr) ;
            }
        }
        for( auto & same_feature_range_it : same_feature_map_ ) {
            std::vector<std::pair<int,int>> & vec_tmp = same_feature_it.second;
            for( auto index_pair : vec_tmp ) {
                int begin = index_pair.first;
                int end = index_pair.end;
                for( int index_i = begin ; index_i < end; index_i++ ) {

                    i = rearrange_elem( index, data_ptr, i, new_data_ptr) ;
                }
            }
        }
        }
    }
    
    //
    void rearrange_all() {
        for( auto std::string data_name : transform_data_set_  ) {
            auto addr_it = name_ptr_map_.find( data_name );
            if( addr_it == name_ptr_map_.end() ) {
                LOG(FATAL) << "can not find the ptr of " << data_name;
            }
            void * addr = addr_it.second;

            auto type_it = name_type_map_.find( data_name );
            if( type_it == name_type_map_.end() ) {
                LOG(FATAL) << "can not find the type of "<< data_name;
            }
            VarType var_type = type_it.second;
            void * data_new;
            if(var_type == var_float_ptr) {
                float * data_new_float = rearrange( (float*)addr );
                data_new = (void*)data_new_float;
            } else if(var_type == var_double_ptr){
                
                double * data_new_double = rearrange( (double*)addr );

                data_new = (void*)data_new_double;
            } else if(var_type == var_int_ptr) { 
                int * data_new_int = rearrange( (int*)addr );

                data_new = (void*)data_new_int;
            } else {
                LOG(FATAL) << "Unsupported";
            }
            name_new_ptr_map_[ data_name ] = data_new;
            name_new_type_map_[gather_index] = var_type;

        }
            for( auto scatter_index : scatter_set_ ) {
                auto scatter_info_it = scatter_map_.find( scatter_index );
                CHECK(scatter_info_it != scatter_map_.end()) << "Can not find "<<scatter_index;
                int * rearrange_scatter_data = rearrange( scatter_info_it->second );
                scatter_name_new_ptr_map_[ scatter_index ] = rearrange_scatter_data;
                /////////////reduction
                auto reduction_info_it = reduction_set_.find( scatter_index );
                if(reduction_info_it != reduction_set_.end()) {
                    int * rearrange_reduction_data = rearrange( reduction_info_it->second );
                    name_new_ptr_map_[ scatter_index ] = rearrange_gather_data;
                    name_new_type_map_[scatter_index] = var_int_ptr;
                    reduction_name_new_ptr_map_[scatter_index] = rearrange_reduction_data;
                    gather_name_new_ptr_map_[scatter_index] = rearrange_scatter_data;
                }
            }
            for( auto gather_index : gather_set_ ) {
                auto gather_info_it = gather_map_.find( gather_index );
                CHECK(gather_info_it != scatter_map_.end()) << "Can not find "<<scatter_index;
                int * rearrange_gather_data = rearrange( gather_info_it->second );
    
                gather_name_new_ptr_map_[gather_index] = rearrange_gather_data;
            }
    }
}

void transform_data(
    const std::set<std::string> & transform_data_set,
    const std::map<std::string, VarType > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map
        ) {
    TransformData transform_data( 
            transform_data_set,
            name_ptr_map,
            name_ptr_map,
            gather_set,
            scatter_set,
            reduction_set,
            same_feature_map,
            same_feature_range_map,
            gather_name_new_ptr_map,
            reduction_name_new_ptr_map,
            scatter_name_new_ptr_map,
            name_new_ptr_map
            ).rearrange_all();
}

