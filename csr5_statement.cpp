#include "csr5_statement.hpp"
#include "type.hpp"
#include "statement_print.hpp"
    
    PageRankStateMent::PageRankStateMent(  ) {
        //(*func_)( y_ptr, x_ptr, data_ptr, column_ptr, row_ptr, row_num, row_ptr_ptr_, index_ptr_ptr_ );
        //
        //
        bool true_vec[VECTOR] ;
        double dzero_vec[VECTOR];
        int one_vec[VECTOR];
        for( int i = 0 ; i < VECTOR ; i++ ) {
	 	    true_vec[i] = true;
            dzero_vec[i] = 0;
            one_vec[i] = 1;
	    }

        zero_const_ = new Const(0);

        one_const_ = new Const(1);

        true_vec_const_ = new Const ( true_vec, VECTOR );
        dzero_vec_const_ = new Const( dzero_vec , VECTOR );

        one_vec_const_ = new Const( one_vec , VECTOR );
        int zero_dv[ VECTOR * 2];
        for( int i = 0 ; i < VECTOR * 2 ; i++ )
            zero_dv[i] = 0;
         int shuffle_forward[VECTOR] = {0,1,2,3,4,5,6,7};
        int shuffle_backward[VECTOR] = {8,9,10,11,12,13,14,15};
        shuffle_forward_const_ = new Const(shuffle_forward,VECTOR);
        shuffle_backward_const_ = new Const(shuffle_backward,VECTOR);
        zero_dv_const_ = new Const( zero_dv, VECTOR * 2 );
  


        std::vector<Type> args_type = {__double_ptr,__double_ptr, __double_ptr, __int_ptr,__int_ptr,__int, __int_ptr_ptr, __int_ptr_ptr,__double_ptr_ptr  };
        Type ret_type = __int; 
        func_state_ptr_ = new FuncStatement( __int, args_type );

        y_ptr_var_ =  (*func_state_ptr_->get_args())[0];
        x_ptr_var_ =  (*func_state_ptr_->get_args())[1];
//        data_ptr_var_ = (*func_state_ptr_->get_args())[2];
//        column_ptr_var_ = (*func_state_ptr_->get_args())[3];
        row_ptr_var_ = (*func_state_ptr_->get_args() )[4];
        row_num_var_ = (*func_state_ptr_->get_args())[5];
        row_ptr_ptr_var_ = (*func_state_ptr_->get_args())[6];
        column_ptr_ptr_var_ = (*func_state_ptr_->get_args())[7];
        data_ptr_ptr_var_ = (*func_state_ptr_->get_args())[8];

        y_v_ptr_var_ = new Varience( __double_v_ptr );
        x_v_ptr_var_ = new Varience( __double_v_ptr );
 //       data_v_ptr_var_ = new Varience( __double_v_ptr );
//        column_v_ptr_var_ = new Varience( __int_v_ptr );
        
        y_ptr_v_var_ = new Varience( __double_ptr_v );
        x_ptr_v_var_ = new Varience( __double_ptr_v );

///////////////////init
        init_state_ = LetStat::make( y_v_ptr_var_, BitCast::make( y_ptr_var_ , __double_v_ptr ) );
        init_state_ = Block::make(init_state_, LetStat::make( x_v_ptr_var_ , BitCast::make( x_ptr_var_, __double_v_ptr) ));
//        init_state_ = Block::make(init_state_, LetStat::make( data_v_ptr_var_ , BitCast::make( data_ptr_var_, __double_v_ptr) )); 
//        init_state_ = Block::make(init_state_, LetStat::make( column_v_ptr_var_ , BitCast::make( column_ptr_var_, __int_v_ptr) ));
////////////////broadcast
        init_state_ = Block::make(init_state_,LetStat::make( x_ptr_v_var_, BroadCast::make( x_ptr_var_) ));
        init_state_ = Block::make(init_state_,LetStat::make( y_ptr_v_var_, BroadCast::make( y_ptr_var_) ));

    };
    void PageRankStateMent::make(const std::map<Mask2 , int> & mask_num_map )  {
        Const * const_zero = new Const(0);

        Const * const_one = new Const(1);
        int i = 0;
        for( auto it : mask_num_map ) {

           const Mask2 & mask =  it.first;
           const int circle_num = it.second;
//           Varience * mask_print = new Varience(__int);

//           Varience * num_print = new Varience(__int);
//           Const * mask_const = new Const(mask.mask_);
//           Const * num_const = new Const( mask.num_);
//           state_vec_.push_back(LetStat::make( mask_print,mask_const));
//           state_vec_.push_back(Print::make( mask_print ));

//           state_vec_.push_back(LetStat::make( num_print,num_const));
            
//           state_vec_.push_back(Print::make( num_print ));

           StateMent * state ;
//           if( mask.mask_ != 0x1  ) {
           if( mask.can_block() ) {
               state = GenerateMaskState( mask, circle_num, i );
           } else {
               state = GenerateReduceState( mask , circle_num ,i );
           } 
           state_vec_.push_back( state );
           i++;
        }
    }
    FuncStatement * PageRankStateMent::get_function() {
        StateMent * state = CombinStatVec(state_vec_);
        state = Block::make( init_state_ , state );
        func_state_ptr_->set_state(state);
        return func_state_ptr_;
    }


