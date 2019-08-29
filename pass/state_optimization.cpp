
#include "bit2addr.h"
StateMent * OptimizationPass::pass_(Block * stat) {
    StateMent * block_state = StateMentPass::pass(stat);
    return For::make( new Const(0), new Const(1), new Const( vec_num ),  block_state ); 
}
StateMent * OptimizationPass::pass_(Gather * stat) {
    StateMent * var_name = stat->var_name_;
    auto gather_map_it = gather_map_.find(var_name);
    if( gather_map_it != gather_map_.end() ) {
        GatherInfo gather_info = (gather_map_it->second)[index_];
        if( gather_info.order_type_ == DisOrder ) {
            int mask = gather_info.get_mask() & VEC_MASK;
            if( mask == 1 ) {
                Varience * gather_info_var = gather_info_var_map_[var_name];
                Varience * load_index = new Varience( __int );

                Varience * permulation_addr = new Varience( __int64 );
                index_state = LetStat::make( load_index , Load::make( gather_info_var ) );
                LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(1) ) ) 
                
                LetStat::make( permulation_addr , Load::make(BitCast::make( gather_info_var, __int64_ptr )) ); 
    
                LetStat::make( gather_info_var , IncAddr::make( gather_info_var, new Const(2) ) );
////uncompleted                
                LetStat::make( x_simd_tmp , Load::make( BitCast::make( x_ptr ,__double_v_ptr )  )  );
                LetStat::make( x_simd , Shuffle::make( x_simd_tmp , permulation_addr) );
            } else {
                return StateMentPass::pass(stat); 
            }
        } else if ( gather_info.order_type_ == IncContinue)  {
            int mask = scatter_info.get_mask() & VEC_MASK;
            Bit2Addr bit2addr( VECTOR);
            CompressAddr compress_addr = bit2addr.generate_compress( mask );

            Const * compress_mem_mask_const = new Const( compress_addr.mask_);


            store_state = Load::make( y_v_addr ,add_simd .compress_mem_mask_const) ;

        } else {
            LOG(FATAL) << "does not supported";
        }
    } else {
        //doesn't have gather information, not optimize it
        return StateMentPass::pass(stat); 
    }
}
StateMent * OptimizationPass::pass_(Add * stat ) { 
        StateMent * var_name = stat->var_name_;
        auto reduction_map_it = reduction_map_.find(var_name);
        if( reduction_map_it != reduction_map_.end() ) {
            StateMent * v1_state_new = pass(v1_state);
            StateMent * v2_state_new = pass(v2_state);
            StateMent * add_new_state;
            Varience * shuffle_res = new Varience(v2_state_new->get_type()); 
            add_new_state = LetStat::make( shuffle_res, v2_state_new);
            ReductionInfo reduction_info = (reduction_map_it->second)[index_];
            if(reduction_info.order_type == IncContinue || reduction_info.order_type_ ==Equel) {
                int mask = reduction_info.mask_&VEC_MASK;
                Bit2Addr bit2addr(VECTOR);
                trans_addr_ = bit2addr.generate( mask );
                int * reduce_addr_int = (int*)malloc(sizeof(int)* VECTOR * reduce_num );
                const char * reduce_addr = trans_addr.addr;
                for( int i = 0 ; i < reduce_num ; i++ ) {
                    for( int j = 0 ; j < VECTOR ; j++ ) {
                        reduce_addr_int[ i* VECTOR + j ] = (int)reduce_addr[ i * VECTOR + j ];
                    }
                }
                for( int i = 0 ; i < reduce_num ; i++ )  {
                    shuffle_index_const_vec.push_back( new Const( reduce_addr_int+ i*VECTOR, VECTOR ) );
                }
                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {

                    Varience * shuffle_simd = new Varience( __double_v);
                    StateMent * shuffle_state = LetStat::make( shuffle_simd, Shuffle::make( shuffle_res, dzero_vec_const_, shuffle_index_const_vec[reduce_i] )); 
                    StateMent * add_state = LetStat::make( shuffle_res, Add::make( shuffle_simd, shuffle_res ) );

                }
                Bit2Addr bit2addr( VECTOR);
                CompressAddr compress_addr = bit2addr.generate_compress( mask );

                Const * compress_const = new Const( compress_addr.compress_vec, VECTOR );


                StateMent * compress_state = LetStat::make( compress_simd, Shuffle::make( shuffle_res , dzero_vec_const_, compress_const ) );

                StateMent * add_y_state = LetStat::make( add_simd, Add::make( compress_simd, y_simd ) );
            } else {
                LOG(FATAL) << "Unsupported";
            }
        } else {
            return StateMentPass::pass_(stat); 
        }
}

StateMent * OptimizationPass::pass_(Scatter * stat) {
    
    StateMent * new_stat = StateMentPass::pass_(stat); 
    StateMent * var_name = stat->var_name_;
    auto scatter_map_it = scatter_map_.find(var_name);
    if( scatter_map_it != reduction_map_.end() ) {
        
        StateMent * addr_state = new_stat->get_addr();
        StateMent * data_state = new_stat->get_data();
        ScatterInfo scatter_info = scatter_map_it->second;
        if(scatter_info.order_type_ == IncContinue)  {
            int mask = scatter_info.get_mask() & VEC_MASK;
            Bit2Addr bit2addr( VECTOR);
            CompressAddr compress_addr = bit2addr.generate_compress( mask );

            Const * compress_mem_mask_const = new Const( compress_addr.mask_);


            store_state = Store::make( y_v_addr ,add_simd .compress_mem_mask_const) ;

        } else {
            LOG(FATAL)  << "Unsupported";
        }
    } else {
        return new_stat;
    }    
}


StateMent * OptimizationInnerReducePass::pass_(Block * stat) {
    StateMent * block_state = StateMentPass::pass(stat);
    
    /////assume that there is only one scatter/store operation
    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    std::vector<StateMent* > * inner_state_vec = new std::vector<StateMent*>() ;
;
    std::vector<StateMent* > * outer_state_vec = new std::vector<StateMent*>() ;
    for( auto state : state_vec ) {
        StateMent * state_new = pass(state);
        if(state != state_new && 
            ( dynamic_cast<Scatter*>( state) != NULL ||
              dynamic_cast<Add*>(state) != NULL)
          ) {
            outer_state_vec.push_back(state);
        } else {
            inner_state_vec.push_back(state_new);
        }
    }
    return For::make( new Const(0), new Const(1), new Const( vec_num ),  block_state ); 
}

