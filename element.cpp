#include "csr5_statement.hpp"
#include "bit2addr.h"
StateMent * PageRankStateMent::get_block_element( Varience * column_v_ptr, Varience * data_v_ptr , Varience * row_v_ptr ,const Mask2 & mask_para ) {
        const int circle_num = mask_para.num_;
        const int circle_mask = mask_para.mask_;

        Varience * row_simd = new Varience( __int_v );

        Varience * res_simd = new Varience(__double_v);
        std::vector<StateMent *> state_vec_tmp;
        StateMent * res_state = LetStat::make( res_simd , dzero_vec_const_ ,false);
        Varience * x_simd[VECTOR];
        int zero_dv[ VECTOR * 2];
        for( int i = 0 ; i < VECTOR * 2 ; i++ )
            zero_dv[i] = 0;
        
        StateMent * gather_state_ptr[VECTOR];
        StateMent * column_state_ptr[VECTOR];
        Varience * column_dsimd ;
        int shuffle_forward[VECTOR] = {0,1,2,3,4,5,6,7};
        int shuffle_backward[VECTOR] = {8,9,10,11,12,13,14,15};
        Const * shuffle_forward_const = new Const(shuffle_forward,VECTOR);
        Const * shuffle_backward_const = new Const(shuffle_backward,VECTOR);
        Const * zero_dv_const = new Const( zero_dv, VECTOR * 2 );
        for( int i = 0 ; i < VECTOR ; i++ ) {
            
            Const * i_const = new Const( i);
            Varience * column_simd = new Varience( __int_v);

            x_simd[i] = new Varience(__double_v);
            if( i % 2 == 0 ) {
                column_dsimd = new Varience(__int_dv);
                StateMent * load_col_state_tmp = LetStat::make( column_dsimd, Load::make( BitCast::make(IncAddr::make(column_v_ptr, i_const),__int_dv_ptr),true ) );
                StateMent * shuffle_col_state_tmp = LetStat::make( column_simd,Shuffle::make( column_dsimd,zero_dv_const, shuffle_forward_const ) );
                column_state_ptr[i] = Block::make( load_col_state_tmp, shuffle_col_state_tmp );
            } else {
            
                column_state_ptr[i] = LetStat::make( column_simd,Shuffle::make( column_dsimd,zero_dv_const, shuffle_backward_const ) );
            }
            gather_state_ptr[i] = LetStat::make( x_simd[i] , Gather::make( x_ptr_v_var_, column_simd , true_vec_const_ )  );
//            state_vec_tmp.push_back(column_state);

//            state_vec_tmp.push_back(gather_state);

        }

        state_vec_tmp.push_back(res_state);

        #define STEP 1
        for( int i = 0 ; i < STEP ; i++ ) {
            state_vec_tmp.push_back(column_state_ptr[i]);
            state_vec_tmp.push_back(gather_state_ptr[i]);
        }
        bool is_first = true;
        for( int i = 0 ; i < VECTOR ; i++ ) {
            Varience * data_simd = new Varience(__double_v);
//            Varience * row_simd = new Varience( __int_v );

            Const * i_const = new Const( i);
            if( i != 0 && (circle_mask & ( 1 << i )) != 0 ) {
                StateMent * row_state_tmp;
                if(is_first) {

                    row_state_tmp = LetStat::make( row_simd, Load::make( row_v_ptr,true ),false ); 
                    is_first = false;
                } else {
                    row_state_tmp = LetStat::make( row_simd, Add::make(row_simd,one_vec_const_),false );
                }
                state_vec_tmp.push_back( row_state_tmp );
                Varience * y_simd = new Varience(__double_v);
                StateMent * gather_state_tmp = LetStat::make( y_simd ,Gather::make( y_ptr_v_var_, row_simd  , true_vec_const_ ));
                StateMent * scatter_state_tmp = Scatter::make( y_ptr_v_var_, row_simd , Add::make(y_simd,res_simd) , true_vec_const_ );
                 
                state_vec_tmp.push_back( gather_state_tmp );
                state_vec_tmp.push_back( scatter_state_tmp );

                StateMent * res_state = LetStat::make( res_simd , dzero_vec_const_ ,false);

                state_vec_tmp.push_back( res_state);
            }
            if( i < VECTOR - STEP ) {
                state_vec_tmp.push_back(column_state_ptr[i+STEP]);
                state_vec_tmp.push_back(gather_state_ptr[i+STEP]);
            }

            StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make(data_v_ptr,i_const),true ) );

                    
            StateMent * res_state = LetStat::make( res_simd , Add::make( Mul::make( x_simd[i] , data_simd ),res_simd ),false);
            
            state_vec_tmp.push_back(data_state);
            state_vec_tmp.push_back(res_state);

        }
        StateMent * row_state_tmp;
        if(is_first) {

            row_state_tmp = LetStat::make( row_simd, Load::make( row_v_ptr,true ),false ); 
            is_first = false;
        } else {
            row_state_tmp = LetStat::make( row_simd, Add::make(row_simd,one_vec_const_),false );
        }
        Varience * y_simd = new Varience(__double_v);
        StateMent * gather_state_tmp = LetStat::make( y_simd ,Gather::make( y_ptr_v_var_, row_simd  , true_vec_const_ ));

        state_vec_tmp.push_back( row_state_tmp );

        state_vec_tmp.push_back( gather_state_tmp );
        StateMent * scatter_state_tmp = Scatter::make( y_ptr_v_var_, row_simd , Add::make(y_simd,res_simd) , true_vec_const_ );
        state_vec_tmp.push_back( scatter_state_tmp );
        return CombinStatVec( state_vec_tmp );
}
StateMent * PageRankStateMent::get_element(Varience * column_ptr_inc_var, Varience * data_ptr_inc_var , Varience * y_addr ,const Mask2 &mask_para,const TransAddr& trans_addr) {
        const int circle_num = mask_para.num_;
        const int circle_mask = mask_para.mask_;
               
        const int reduce_num = trans_addr.num;
        const char * reduce_addr = trans_addr.addr;
        Const * const_end = new Const( circle_num );

        int compress_vec[VECTOR];
        bool compress_mask_vec[VECTOR];
        int ii = 0; 
        for( int i = 0; i < VECTOR ; i++ ) {
            if(( (1<<i) & circle_mask ) != 0) {
                compress_vec[ii] = i;
                compress_mask_vec[ii] = true;
                ii++;
            }
        }
        int compress_mem_mask = 0xff;
        compress_mem_mask >>= (VECTOR - ii);
        for( ; ii < VECTOR ; ii++ ) {
            compress_vec[ii] = VECTOR;
            compress_mask_vec[ii] = false;
        }
        Const * compress_mem_mask_const = new Const( compress_mem_mask );
        Const * compress_const = new Const( compress_vec, VECTOR );
        Const * compress_mask_const = new Const( compress_mask_vec , VECTOR );

        int * reduce_addr_int = (int*)malloc(sizeof(int)* VECTOR * reduce_num );
        
        for( int i = 0 ; i < reduce_num ; i++ ) {
            for( int j = 0 ; j < VECTOR ; j++ ) {
                reduce_addr_int[ i* VECTOR + j ] = (int)reduce_addr[ i * VECTOR + j ];
            }
        }

        std::vector<Const *> shuffle_index_const_vec;
        for( int i = 0 ; i < reduce_num ; i++ )  {
            shuffle_index_const_vec.push_back( new Const( reduce_addr_int+ i*VECTOR, VECTOR ) );
        }

        Const * Four = new Const(4);

            Varience * x_simd = new Varience( __double_v );

            Varience * mul_simd = new Varience( __double_v );

            Varience * shuffle_simd = new Varience( __double_v );
            Varience * compress_simd = new Varience( __double_v );
            Varience * y_simd = new Varience( __double_v );

            Varience * add_simd = new Varience( __double_v );
            Varience * data_simd = new Varience( __double_v );
            Varience * index_simd = new Varience( __int_v );
            Varience * y_v_addr = new Varience(__double_v_ptr);
            std::vector<StateMent*> state_vec;

//            StateMent * print_state = Print::make(  dynamic_cast<Varience*>(index) );
            if(  circle_mask != 0x1 ) {

                StateMent * data_state = LetStat::make( data_simd, Load::make( data_ptr_inc_var ,true ) ); 

                StateMent * index_state = LetStat::make( index_simd, Load::make( column_ptr_inc_var ,true ) ); 
            
                StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const_ )  ); 


                StateMent * mul_state = LetStat::make(mul_simd,  Mul::make( data_simd , x_simd ),false );  
                
                state_vec.push_back( data_state );state_vec.push_back( index_state );
                state_vec.push_back(x_state); state_vec.push_back(mul_state);


                StateMent * complex_reduce_state;
                StateMent * detect_state ;
                StateMent * calc_state;

                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {
                    StateMent * shuffle_state = LetStat::make( shuffle_simd, Shuffle::make( mul_simd,dzero_vec_const_, shuffle_index_const_vec[reduce_i] ),false); 
                    StateMent * add_state = LetStat::make( mul_simd, Add::make( shuffle_simd,mul_simd ) ,false);

                    state_vec.push_back( shuffle_state );state_vec.push_back( add_state );
                }
                StateMent * y_v_addr_state = LetStat::make( y_v_addr , BitCast::make( y_addr, __double_v_ptr ));
                StateMent * y_state;
                if(reduce_num == 0) {
                     
                    y_state = LetStat::make( y_simd, Load::make( y_v_addr ) ); 
                } else {
                    y_state = LetStat::make( y_simd, Load::make( y_v_addr, compress_mask_const ) ); 
                }
                StateMent * compress_state = LetStat::make( compress_simd, Shuffle::make( mul_simd, dzero_vec_const_, compress_const ) );
                 

                StateMent * add_y_state = LetStat::make( add_simd, Add::make( compress_simd, y_simd ) );

                 

                
                StateMent * store_state ;
                if( reduce_num == 0 )
                    store_state = Store::make( y_v_addr ,add_simd ) ;
                else 
                    store_state = Store::make( y_v_addr ,add_simd , compress_mask_const ) ;
                
                state_vec.push_back( y_v_addr_state ); 
                state_vec.push_back( y_state );
                state_vec.push_back( compress_state );
                state_vec.push_back( add_y_state );


//                state_vec.push_back( print_state );
                state_vec.push_back( store_state );

//                state_vec.push_back( print_state );
                 
            } else {
                Varience * new_index = new Varience( __int );
                Varience * fast_res = new  Varience( __double_v );
                Varience * y_data = new Varience(__double);

                StateMent * y_addr_state = LetStat::make(y_data , Load::make( y_addr ));
                state_vec.push_back(y_addr_state);
                if(circle_num == 1){
                    StateMent * data_state = LetStat::make( data_simd, Load::make( data_ptr_inc_var,true ) ); 

                    StateMent * index_state = LetStat::make( index_simd, Load::make( column_ptr_inc_var ,true ) ); 
            
                    StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const_ )  ); 

            
                    StateMent * fmadd_state = LetStat::make(fast_res, Mul::make( data_simd , x_simd ) ,false);  
                    state_vec.push_back( data_state );

                    state_vec.push_back( index_state );
                    state_vec.push_back( x_state );
                    state_vec.push_back( fmadd_state );
                } else {

                    StateMent * init_state = LetStat::make( fast_res , dzero_vec_const_,false);
                    StateMent * for_state = For::make( zero_const_ , one_const_, const_end);
                     
                    StateMent * inc_i = dynamic_cast<For*>(for_state)->get_var();
                    StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make( data_ptr_inc_var , inc_i ),true ) ); 

                    StateMent * index_state = LetStat::make( index_simd, Load::make( IncAddr::make( column_ptr_inc_var, inc_i ),true ) ); 
            
                    StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const_ )  ); 

            
                    StateMent * fmadd_state = LetStat::make(fast_res,  Add::make(fast_res, Mul::make( data_simd , x_simd ) ),false);  
                
                    std::vector<StateMent*> for_state_vec = {  data_state, index_state, x_state, fmadd_state };
                    StateMent * inner_for_state = CombinStatVec( for_state_vec );
                
                    dynamic_cast<For*>(for_state)->SetState(inner_for_state);
                     
                    state_vec.push_back( init_state );state_vec.push_back( for_state );

                }
                StateMent * store_state = Store::make( y_addr ,Add::make(y_data ,Reduce::make( fast_res )));
                state_vec.push_back(store_state); 

            }
            

            StateMent * res_state = CombinStatVec( state_vec );
            return res_state;
}

StateMent * PageRankStateMent::GenerateInit(int mask_i,int circle_mask) {

    Const * mask_i_const = new Const(mask_i);
    column_ptr_var_get_ = new Varience(__int_ptr);
    row_ptr_var_get_ = new Varience( __int_ptr );
    data_ptr_var_get_ = new Varience( __double_ptr );
    column_v_ptr_var_get_ = new Varience(__int_v_ptr);
    row_v_ptr_var_get_ = new Varience( __int_v_ptr );
    data_v_ptr_var_get_ = new Varience( __double_v_ptr );


    StateMent * column_ptr_state = LetStat::make( column_ptr_var_get_ , Load::make(IncAddr::make( column_ptr_ptr_var_ , mask_i_const)) );

    StateMent * data_ptr_state = LetStat::make( data_ptr_var_get_ , Load::make(IncAddr::make( data_ptr_ptr_var_ , mask_i_const)) );

    StateMent * row_ptr_state = LetStat::make( row_ptr_var_get_, Load::make(IncAddr::make( row_ptr_ptr_var_, mask_i_const )) );


    StateMent * column_v_ptr_state = LetStat::make( column_v_ptr_var_get_ , BitCast::make( column_ptr_var_get_ , __int_v_ptr)); 
    
    StateMent * row_v_ptr_state = LetStat::make( row_v_ptr_var_get_ , BitCast::make( row_ptr_var_get_ , __int_v_ptr));

    StateMent * data_v_ptr_state = LetStat::make( data_v_ptr_var_get_ , BitCast::make( data_ptr_var_get_ , __double_v_ptr)); 
    std::vector<StateMent*> state_vec_tmp = { column_ptr_state,data_ptr_state,row_ptr_state , column_v_ptr_state, data_v_ptr_state,row_v_ptr_state};
/////////////////////////////////
    Bit2Addr bit2addr(VECTOR);
    trans_addr_ = bit2addr.generate( circle_mask );

    return CombinStatVec(state_vec_tmp);
}

StateMent * PageRankStateMent::GenerateReduceState(const Mask2 & mask, const int circle_num, const int mask_i) {
    const int inner_circle_num = mask.num_;
    const int circle_mask = mask.mask_;
    Varience * column_v_ptr_inc_var = new Varience(__int_v_ptr);

    Varience * row_v_ptr_inc_var = new Varience(__int_v_ptr);
    Varience * data_v_ptr_inc_var = new Varience(__double_v_ptr);
    Varience * y_offset_ptr = new Varience(__double_ptr);
   
    Varience * block_for_i = new Varience(__int);
    Const * const_end = new Const( circle_num );
    Varience * row_index = new Varience(__int);
    StateMent * init_state = GenerateInit(mask_i,circle_mask);


    StateMent * push_state = For::make(zero_const_ , one_const_, const_end);

    StateMent * inc_i = dynamic_cast<For*>(push_state)->get_var();

   
    StateMent * row_index_state = LetStat::make( row_index , Load::make( IncAddr::make(row_ptr_var_get_, inc_i) )) ;

    StateMent * block_for_i_state = LetStat::make( block_for_i , Mul::make ( inc_i, new Const(inner_circle_num)) );

    StateMent * y_addr_state = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );
    StateMent * column_ptr_var_inc_state = LetStat::make( column_v_ptr_inc_var, IncAddr::make( column_v_ptr_var_get_, block_for_i) ) ;

    StateMent * data_ptr_var_inc_state = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, block_for_i) ) ;

    
    StateMent * element_state = get_element( column_v_ptr_inc_var , data_v_ptr_inc_var , y_offset_ptr , mask , trans_addr_);
    
    std::vector<StateMent*> inner_for_vec = { block_for_i_state, row_index_state,y_addr_state,column_ptr_var_inc_state,data_ptr_var_inc_state,element_state };
    dynamic_cast<For*>(push_state)->SetState( CombinStatVec( inner_for_vec ) );
    return Block::make( init_state,push_state );
}

StateMent * PageRankStateMent::GenerateMaskState( const Mask2 & mask , const int circle_num , const int mask_i ) {

    const int circle_mask = mask.mask_;
    Const * mask_i_const = new Const(mask_i);
    const int circle_block_num = circle_num / VECTOR;
    const int circle_single_num = circle_num % VECTOR;
    Const * const_end = new Const( circle_block_num );

    Varience * block_for_i = new Varience(__int);
    Varience * column_ptr_inc_var = new Varience(__int_ptr);

    Varience * row_ptr_inc_var = new Varience(__int_ptr);
    Varience * data_ptr_inc_var = new Varience(__double_ptr);

    Varience * column_v_ptr_inc_var = new Varience(__int_v_ptr);

    Varience * row_v_ptr_inc_var = new Varience(__int_v_ptr);
    Varience * data_v_ptr_inc_var = new Varience(__double_v_ptr);

    
    StateMent * init_state = GenerateInit(mask_i,circle_mask);

    std::vector<StateMent*> state_vec_tmp = {init_state};
    if(circle_block_num > 0) {   
        StateMent * block_for_state = For::make(zero_const_ , one_const_, const_end);

        StateMent * get_block_for_i = dynamic_cast<For*>(block_for_state)->get_var();
        StateMent * block_for_i_state = LetStat::make( block_for_i , Mul::make (get_block_for_i, new Const(VECTOR)) );

        StateMent * column_ptr_var_inc_state = LetStat::make( column_v_ptr_inc_var, IncAddr::make( column_v_ptr_var_get_, block_for_i) ) ;

        StateMent * data_ptr_var_inc_state = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, block_for_i) ) ;

        StateMent * row_ptr_var_inc_state = LetStat::make( row_v_ptr_inc_var, IncAddr::make( row_v_ptr_var_get_, get_block_for_i) ) ;

        StateMent * block_state =  get_block_element( column_v_ptr_inc_var, data_v_ptr_inc_var , row_v_ptr_inc_var ,  mask ) ;
        std::vector<StateMent *> inner_for_vec_tmp = { block_for_i_state, column_ptr_var_inc_state, data_ptr_var_inc_state, row_ptr_var_inc_state,block_state  };
        StateMent * inner_for_tmp = CombinStatVec( inner_for_vec_tmp );
        dynamic_cast<For*> (block_for_state)->SetState( inner_for_tmp );
        state_vec_tmp.push_back(block_for_state);
    }
//    Const * num_const = new Const(-1);

//           Varience * num_print = new Varience(__int);
//    state_vec_.push_back(LetStat::make( num_print,num_const));
            
//    state_vec_.push_back(Print::make( num_print ));

    //////////////special

    for( int i = 0 ; i < circle_single_num ; i++ ) {
        const int begin_i = i + circle_block_num * VECTOR;
        Varience * y_offset_ptr = new Varience( __double_ptr);
        Const * begin_i_const = new Const( begin_i );
        
        column_v_ptr_inc_var = new Varience( __int_v_ptr ); 
        data_v_ptr_inc_var = new Varience( __double_v_ptr );
//        row_ptr_inc_var = new Varience( __int_ptr);
        Varience * row_index = new Varience( __int); 
        StateMent * column_ptr_var_inc_state_spec = LetStat::make( column_v_ptr_inc_var, IncAddr::make( column_v_ptr_var_get_, begin_i_const) ) ;
        StateMent * data_ptr_var_inc_state_spec = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, begin_i_const) ) ;
        
        StateMent * row_index_state_spec = LetStat::make( row_index , Load::make( IncAddr::make(row_ptr_var_get_ ,begin_i_const) )) ;
        
        StateMent * y_addr_state_spec = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );

        StateMent * element_state_spec = get_element( column_v_ptr_inc_var , data_v_ptr_inc_var , y_offset_ptr , mask , trans_addr_) ;

        state_vec_tmp.push_back( column_ptr_var_inc_state_spec );
        state_vec_tmp.push_back( data_ptr_var_inc_state_spec );
        state_vec_tmp.push_back( row_index_state_spec );
        state_vec_tmp.push_back( y_addr_state_spec );
        state_vec_tmp.push_back( element_state_spec );
    }
    return CombinStatVec(state_vec_tmp);    
}












