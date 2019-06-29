#include "csr5_statement.hpp"
#include "type.hpp"
#include "statement_print.hpp"
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
        std::vector<Type> args_type = {__float_ptr, __int_ptr, __int_ptr, __float_ptr,__int_ptr,__int8_ptr,__int_ptr_ptr};
        Type ret_type = __int; 
        func_state_ptr_ = new FuncStatement( __int, args_type );

        sum_var_ =  (*func_state_ptr_->get_args())[0];
        n1_var_ =  (*func_state_ptr_->get_args())[1];
        n2_var_ =  (*func_state_ptr_->get_args())[2];
        rank_var_ =  (*func_state_ptr_->get_args())[3];
        nneibor_var_ =  (*func_state_ptr_->get_args())[4];

        shuffle_index_var_ =  (*func_state_ptr_->get_args())[5];

        mask_addr_ =  (*func_state_ptr_->get_args())[6];


      
        sum_var_v_ = new Varience( __float_v_ptr );

        n1_var_v_ = new Varience( __int_v_ptr );
        n2_var_v_ = new Varience( __int_v_ptr );
        rank_var_v_ = new Varience( __float_v_ptr );
        nneibor_var_v_ = new Varience( __int_v_ptr );
        shuffle_index_var_v_ = new Varience( __int8_v_ptr );

        sum_ptr_v_ = new Varience( __float_ptr_v );
        rank_ptr_v_ = new Varience( __float_ptr_v );
        nneibor_ptr_v_ = new Varience( __int_ptr_v );

        init_state_ = LetStat::make( sum_var_v_, BitCast::make( sum_var_, __float_v_ptr ) );

        init_state_ = Block::make(init_state_, LetStat::make( n1_var_v_ , BitCast::make( n1_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( n2_var_v_ , BitCast::make( n2_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( rank_var_v_ , BitCast::make( rank_var_, __float_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( nneibor_var_v_, BitCast::make( nneibor_var_, __int_v_ptr) ));

        init_state_ = Block::make(init_state_, LetStat::make( shuffle_index_var_v_, BitCast::make( shuffle_index_var_, __int8_v_ptr) ));
        init_state_ = Block::make(init_state_,LetStat::make( sum_ptr_v_, BroadCast::make( sum_var_) ));


        init_state_ = Block::make(init_state_,LetStat::make( rank_ptr_v_, BroadCast::make( rank_var_) ));

        init_state_ = Block::make(init_state_,LetStat::make( nneibor_ptr_v_, BroadCast::make( nneibor_var_ )));
    };
void PageRankStateMent::make( int * shuffle_num_vec,int mask_num, int * addr_num )  {

       Const * const_zero = new Const(0);

       Const * const_one = new Const(1);

       for( int mask = 0 ; mask < MASK_NUM ; mask++ ) {
           Const * const_end = new Const( addr_num[mask] );
           Const * mask_const = new Const( mask );
           Varience * index = new Varience(__int);
           Varience * addr_ptr = new Varience(__int_ptr);
            
           StateMent * addr_ptr_state = LetStat::make( addr_ptr, Load::make(IncAddr::make( mask_addr_, mask_const )) ); 
           StateMent * push_state = For::make(const_zero , const_one, const_end); 

           StateMent * inc_i = dynamic_cast<For*>(push_state)->get_var();
           StateMent * index_state = LetStat::make( index , Load::make(IncAddr::make( addr_ptr ,inc_i)) );

//           StateMent * index_state = LetStat::make( index , const_zero );
           StateMent * elem_state = get_element( index, mask);
           dynamic_cast<For*>(push_state)->SetState(Block::make(index_state,elem_state));
           push_state = Block::make( addr_ptr_state, push_state);
           state_vec_.push_back( push_state );
       }
    }
    FuncStatement * PageRankStateMent::get_function() {
        LOG(INFO) << state_vec_.size();
        StateMent * state = CombinStatVec(state_vec_);
        state = Block::make( init_state_ , state );
        func_state_ptr_->set_state(state);
        return func_state_ptr_;
    }


