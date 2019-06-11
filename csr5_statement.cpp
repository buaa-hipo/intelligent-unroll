#include "csr5_statement.hpp"
#include "type.hpp"
    StateMent* CombinStatVec( const std::vector<StateMent*> &stat_vec ) {
        if( stat_vec.size() == 0 ) { 
		    return Nop::make(); 
	    }
        else if(stat_vec.size() == 1) {return stat_vec[0];}
        else {
            StateMent * block_stat = Block::make( stat_vec[0],stat_vec[1]);
            for( int i = 2 ; i < stat_vec.size(); i++ ) {
                block_stat = Block::make( block_stat, stat_vec[i] );
            }
            return block_stat;
        }
    }

    int generate_mask( int * index ) {
       int mask = 0x0; 
       for( int i = 0 ; i < VECTOR ; i++ ) {
            int value = index[i];
            for( int j = 0 ; j < VECTOR ; j++) {
                if( index[j] == value) {
                        if( j < i ) {
                            break;
                        } else {
                            if(j>i) {
                                mask |= 0x1<<j;
                                break;
                            }
                        }
                    
                } 
            }
       }
       return mask;
    }
PageRankStateMent::PageRankStateMent(  ) {
        std::vector<Type> args_type = {__float_ptr, __int_ptr, __int_ptr, __float_ptr,__int_ptr};
        Type ret_type = __int; 
        func_state_ptr_ = new FuncStatement( __int, args_type );

        sum_var_ =  (*func_state_ptr_->get_args())[0];
        n1_var_ =  (*func_state_ptr_->get_args())[1];
        n2_var_ =  (*func_state_ptr_->get_args())[2];
        rank_var_ =  (*func_state_ptr_->get_args())[3];
        nneibor_var_ =  (*func_state_ptr_->get_args())[4];
      
        sum_var_v_ = new Varience( __float_v_ptr );

        n1_var_v_ = new Varience( __int_v_ptr );
        n2_var_v_ = new Varience( __int_v_ptr );
        rank_var_v_ = new Varience( __float_v_ptr );
        nneibor_var_v_ = new Varience( __int_v_ptr );

        sum_ptr_v_ = new Varience( __float_ptr_v );
        rank_ptr_v_ = new Varience( __float_ptr_v );
        nneibor_ptr_v_ = new Varience( __int_ptr_v );

        init_state_ = LetStat::make( sum_var_v_, BitCast::make( sum_var_, __float_v_ptr ) );

        init_state_ = Block::make(init_state_, LetStat::make( n1_var_v_ , BitCast::make( n1_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( n2_var_v_ , BitCast::make( n2_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( rank_var_v_ , BitCast::make( rank_var_, __float_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( nneibor_var_v_, BitCast::make( nneibor_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_,LetStat::make( sum_ptr_v_, BroadCast::make( sum_var_) ));


        init_state_ = Block::make(init_state_,LetStat::make( rank_ptr_v_, BroadCast::make( rank_var_) ));

        init_state_ = Block::make(init_state_,LetStat::make( nneibor_ptr_v_, BroadCast::make( nneibor_var_ )));
    };
void PageRankStateMent::make( PageRankStructurePtr page_rank_structure_ptr )  {
        int * n2 = page_rank_structure_ptr->n2;
        bool true_vec[VECTOR] ;
        for( int i = 0 ; i < VECTOR ; i++ ) {
	 	    true_vec[i] = true;
	    }
        Const * true_vec_const = new Const ( true_vec, VECTOR );
        for( int i = 0 ; i < page_rank_structure_ptr->nedges ; i += VECTOR ) {
            Varience * nx_simd = new Varience( __int_v );
            Varience * ny_simd = new Varience( __int_v );
            Varience * rank_simd = new Varience( __float_v );
            Varience * nneibor_simd = new Varience( __int_v );
            Varience * sum_simd = new Varience( __float_v );

            Varience * div_simd = new Varience( __float_v );
            Varience * sum_simd_new = new Varience( __float_v );

            Varience * complex_reduce_simd = new Varience( __float_v );
            Varience * detect_simd = new Varience( __bool_v );

            Const * index_const = new Const(i/VECTOR);
            StateMent * nx_state = LetStat::make( nx_simd, Load::make( IncAddr::make( n1_var_v_, index_const ) ) ); 

            StateMent * ny_state = LetStat::make( ny_simd, Load::make( IncAddr::make( n2_var_v_, index_const ) ) ); 
            
            StateMent * rank_state = LetStat::make( rank_simd , Gather::make( rank_ptr_v_, nx_simd , true_vec_const )  ); 

            StateMent * nneibor_state = LetStat::make( nneibor_simd , Gather::make( nneibor_ptr_v_, nx_simd , true_vec_const )  ); 
            StateMent * sum_state = LetStat::make( sum_simd , Gather::make( sum_ptr_v_ , ny_simd ,true_vec_const ) );

            StateMent * div_state = LetStat::make(div_simd,  Div::make( rank_simd, BitCast::make(nneibor_simd, __float_v )));  
            int mask = generate_mask( &n2[i] );
            //printf("%x\nmark_grep",mask);
            StateMent * complex_reduce_state;
            StateMent * detect_state ;
            StateMent * calc_state;
            printf("%x\n", mask);
            if( mask != 0x0 ) {
                complex_reduce_state = LetStat::make(complex_reduce_simd, ComplexReduce::make( div_simd ,ny_simd,mask ));

                detect_state = LetStat::make( detect_simd, DetectConflict::make(ny_simd) );

                calc_state = LetStat::make( sum_simd_new, Add::make( sum_simd, complex_reduce_simd) );
            } else {

                calc_state = LetStat::make( sum_simd_new, Add::make( sum_simd, div_simd) );
            }
            StateMent * scatter_state ;
            if(mask != 0x0)
                scatter_state= Scatter::make( sum_ptr_v_ , ny_simd, sum_simd_new , detect_simd );
            else {
                scatter_state= Scatter::make( sum_ptr_v_ , ny_simd, sum_simd_new , true_vec_const );
            }
            
            StateMent * block_state = Block::make( nx_state, ny_state );
            block_state = Block::make( block_state, rank_state );

            block_state = Block::make( block_state, nneibor_state );
            block_state = Block::make( block_state, sum_state );
            block_state = Block::make( block_state, div_state );
            if( mask!=0x0 ) {

                block_state = Block::make( block_state, complex_reduce_state );
                block_state = Block::make( block_state, detect_state );
            }
            block_state = Block::make( block_state, calc_state );
            block_state = Block::make( block_state, scatter_state );
            state_vec_.push_back( block_state );
        } 
    //    exit(0); 
    }
    FuncStatement * PageRankStateMent::get_function() {
        
        StateMent * state = CombinStatVec(state_vec_);
        state = Block::make( init_state_ , state );
        func_state_ptr_->set_state(state);
        return func_state_ptr_;
    }


