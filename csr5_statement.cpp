#include "csr5_statement.hpp"
#include "type.hpp"
#include "statement_print.hpp"
    
    PageRankStateMent::PageRankStateMent(  ) {
        //(*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, index_ptr_ptr_ );
        std::vector<Type> args_type = {__double_ptr,__double_ptr, __double_ptr, __int_ptr,__int_ptr,__int, __int_ptr_ptr, __int_ptr_ptr  };
        Type ret_type = __int; 
        func_state_ptr_ = new FuncStatement( __int, args_type );

        y_ptr_var_ =  (*func_state_ptr_->get_args())[0];
        x_ptr_var_ =  (*func_state_ptr_->get_args())[1];
        data_ptr_var_ = (*func_state_ptr_->get_args())[2];
        column_ptr_var_ = (*func_state_ptr_->get_args())[3];
        row_ptr_var_ = (*func_state_ptr_->get_args() )[4];
        row_num_var_ = (*func_state_ptr_->get_args())[5];
        row_ptr_ptr_var_ = (*func_state_ptr_->get_args())[6];
        index_ptr_ptr_var_ = (*func_state_ptr_->get_args())[7];

        y_v_ptr_var_ = new Varience( __double_v_ptr );
        x_v_ptr_var_ = new Varience( __double_v_ptr );
        data_v_ptr_var_ = new Varience( __double_v_ptr );
        column_v_ptr_var_ = new Varience( __int_v_ptr );
        
        y_ptr_v_var_ = new Varience( __double_ptr_v );
        x_ptr_v_var_ = new Varience( __double_ptr_v );

///////////////////init
        init_state_ = LetStat::make( y_v_ptr_var_, BitCast::make( y_ptr_var_ , __double_v_ptr ) );
        init_state_ = Block::make(init_state_, LetStat::make( x_v_ptr_var_ , BitCast::make( x_ptr_var_, __double_v_ptr) ));
        init_state_ = Block::make(init_state_, LetStat::make( data_v_ptr_var_ , BitCast::make( data_ptr_var_, __double_v_ptr) )); 
        init_state_ = Block::make(init_state_, LetStat::make( column_v_ptr_var_ , BitCast::make( column_ptr_var_, __int_v_ptr) ));
////////////////broadcast
        init_state_ = Block::make(init_state_,LetStat::make( x_ptr_v_var_, BroadCast::make( x_ptr_var_) ));
        init_state_ = Block::make(init_state_,LetStat::make( y_ptr_v_var_, BroadCast::make( y_ptr_var_) ));

    };
    void PageRankStateMent::make(const std::map<Mask2 , int> & mask_num_map )  {
        Const * const_zero = new Const(0);

        Const * const_one = new Const(1);
        int i = 0;
        for( auto it : mask_num_map ) {
        Varience * index_ptr = new Varience(__int_ptr);
        Varience * row_ptr = new Varience( __int_ptr );
        Varience * y_offset_ptr = new Varience( __double_ptr );
        Varience * index = new Varience(__int);
        Varience * row_index = new Varience( __int);

            const Mask2 & mask =  it.first;
            const int circle_num = it.second;
            Const* const_i = new Const( i );
            Const * const_end = new Const( circle_num );

            StateMent * index_ptr_state = LetStat::make( index_ptr, Load::make(IncAddr::make( index_ptr_ptr_var_, const_i )) ); 
            StateMent * row_ptr_state = LetStat::make( row_ptr, Load::make(IncAddr::make( row_ptr_ptr_var_, const_i )) ); 

            if(circle_num == 1) {
                StateMent * index_state = LetStat::make( index , Load::make( index_ptr) );
                StateMent * row_index_state = LetStat::make( row_index , Load::make( row_ptr )) ;
                StateMent * y_addr_state = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );

                StateMent * elem_state = get_element( index, y_offset_ptr, mask ); 
            
                StateMent * inner_for_state = Block::make(y_addr_state,elem_state);
                inner_for_state = Block::make(index_state , inner_for_state );

                inner_for_state = Block::make( row_index_state , inner_for_state );
            
                inner_for_state = Block::make( index_ptr_state, inner_for_state);
                inner_for_state = Block::make( row_ptr_state, inner_for_state);

                state_vec_.push_back( inner_for_state );
            } else {
                StateMent * push_state = For::make(const_zero , const_one, const_end);
                StateMent * inc_i = dynamic_cast<For*>(push_state)->get_var();

                StateMent * index_state = LetStat::make( index , Load::make(IncAddr::make( index_ptr ,inc_i)) );
                StateMent * row_index_state = LetStat::make( row_index , Load::make(IncAddr::make( row_ptr ,inc_i)) );
                StateMent * y_addr_state = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );

                StateMent * elem_state = get_element( index, y_offset_ptr, mask ); 
            
                StateMent * inner_for_state = Block::make(y_addr_state,elem_state);
                inner_for_state = Block::make(index_state , inner_for_state );

                inner_for_state = Block::make( row_index_state , inner_for_state );
                dynamic_cast<For*>(push_state)->SetState(inner_for_state);
            
                push_state = Block::make( index_ptr_state, push_state);
                push_state = Block::make( row_ptr_state, push_state);

                state_vec_.push_back( push_state );
            }
           i++;
        }
    }
    FuncStatement * PageRankStateMent::get_function() {
        LOG(INFO) << state_vec_.size();
        StateMent * state = CombinStatVec(state_vec_);
        state = Block::make( init_state_ , state );
        func_state_ptr_->set_state(state);
        return func_state_ptr_;
    }


