#include "csr5_statement.hpp"
#include "bit2addr.h"
StateMent * PageRankStateMent::get_shuffle_element(Varience * column_ptr_inc_var, Varience * data_ptr_inc_var , Varience * y_addr ,const Mask2 &mask_para,const TransAddr& trans_addr, int load_gather_level , Varience * load_gather_addr) {
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

            Varience * x_simd = new Varience( __double_v );

            Varience * mul_simd = new Varience( __double_v ,false);

            Varience * compress_simd = new Varience( __double_v );
            Varience * y_simd = new Varience( __double_v );

            Varience * add_simd = new Varience( __double_v );
            Varience * data_simd = new Varience( __double_v );
            Varience * y_v_addr = new Varience(__double_v_ptr);
            std::vector<StateMent*> state_vec;


                StateMent * data_state = LetStat::make( data_simd, Load::make( data_ptr_inc_var ,true ) );
                StateMent * index_state;
                StateMent * x_state;
                if(load_gather_level == 1) {
                    
                    Varience * x_ptr = new Varience(__double_ptr); 
                    Varience * x_simd_tmp = new Varience( __double_v );
                    Varience * column_inc_var = new Varience(__int);
                    index_state = LetStat::make( column_inc_var , Load::make(column_ptr_inc_var) );
                    index_state =Block::make( index_state, LetStat::make( x_ptr, IncAddr::make( x_ptr_var_ , column_inc_var ))); 

                    x_state = LetStat::make( x_simd_tmp , Load::make( BitCast::make( x_ptr,__double_v_ptr )  )  );
                    x_state = Block::make( x_state , LetStat::make( x_simd , Shuffle::make( x_simd_tmp , load_gather_addr) ));

                } else {

                    Varience * index_simd = new Varience( __int_v );
                    index_state = LetStat::make( index_simd, Load::make( column_ptr_inc_var ,true ) ); 
            
                    x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const_ )  ); 
               
                }

                StateMent * mul_state = LetStat::make(mul_simd,  Mul::make( data_simd , x_simd ) );  
                
                state_vec.push_back( data_state );state_vec.push_back( index_state );
                state_vec.push_back(x_state); state_vec.push_back(mul_state);


                StateMent * complex_reduce_state;
                StateMent * detect_state ;
                StateMent * calc_state;

                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {

                    Varience * shuffle_simd = new Varience( __double_v);
                    StateMent * shuffle_state = LetStat::make( shuffle_simd, Shuffle::make( mul_simd,dzero_vec_const_, shuffle_index_const_vec[reduce_i] )); 
                    StateMent * add_state = LetStat::make( mul_simd, Add::make( shuffle_simd,mul_simd ) );

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
                 
        return CombinStatVec(state_vec);
}
StateMent * PageRankStateMent::get_reduce_element(Varience * column_ptr_inc_var, Varience * data_ptr_inc_var , Varience * y_addr ,const Mask2 &mask_para , Varience * load_gather_num , Varience * load_gather_addr_ptr) {
        const int circle_num = mask_para.num_;
        const int circle_mask = mask_para.mask_;
               
        Const * const_end = new Const( circle_num );
        Const * Four = new Const(4);

            Varience * x_simd = new Varience( __double_v );

            Varience * mul_simd = new Varience( __double_v );

            Varience * compress_simd = new Varience( __double_v );
            Varience * y_simd = new Varience( __double_v );

            Varience * add_simd = new Varience( __double_v );
            Varience * data_simd = new Varience( __double_v );
            Varience * index_simd = new Varience( __int_v );
            std::string index_name = std::string("column_index");
            Varience * index = new Varience( __int , index_name );
            Varience * y_v_addr = new Varience(__double_v_ptr);


            std::vector<StateMent*> state_vec;

//            StateMent * print_state = Print::make(  dynamic_cast<Varience*>(index) );
                Varience * new_index = new Varience( __int );
                Varience * fast_res = new  Varience( __double_v ,false);
                Varience * y_data = new Varience(__double);

                StateMent * y_addr_state = LetStat::make(y_data , Load::make( y_addr ));
                state_vec.push_back(y_addr_state);

                    StateMent * init_state = LetStat::make( fast_res , dzero_vec_const_);


                    StateMent * load1_gather_for_state = For::make( zero_const_ , one_const_, load_gather_num);

                    StateMent * load1_gather_for_i = dynamic_cast<For*>(load1_gather_for_state)->get_var();
                    StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make( data_ptr_inc_var , load1_gather_for_i ),true ) ); 

                    StateMent * index_state = LetStat::make( index, Load::make( IncAddr::make( BitCast::make(column_ptr_inc_var,__int_ptr), load1_gather_for_i ) ) );
                    Varience * load1_gather_addr = new Varience(__int64  );
                    StateMent * load1_gather_addr_state = LetStat::make( load1_gather_addr , Load::make( IncAddr::make( load_gather_addr_ptr , load1_gather_for_i)  ) );
            
            
                    StateMent * x_state = LetStat::make( x_simd , Shuffle::make(Load::make( BitCast::make(IncAddr::make(x_ptr_var_,index),__double_v_ptr)), load1_gather_addr)  ); 

            
                    StateMent * fmadd_state = LetStat::make(fast_res,  Add::make(fast_res, Mul::make( data_simd , x_simd ) ));  

                    std::vector<StateMent*> inner_load1_gather_for_vec = { data_state, index_state,load1_gather_addr_state,x_state,fmadd_state };
                    dynamic_cast<For*>( load1_gather_for_state)->SetState( CombinStatVec( inner_load1_gather_for_vec ) );
                    
                    state_vec.push_back( init_state );

                    state_vec.push_back(load1_gather_for_state);
///////////////////////////////////////////////////////////////////////////
            Varience * data_simd_gather = new Varience( __double_v );
            Varience * index_simd_gather = new Varience( __int_v );

            Varience * x_simd_gather = new Varience( __double_v );
 
                    StateMent * for_state = For::make( load_gather_num , one_const_, const_end);
                     
                    StateMent * inc_i = dynamic_cast<For*>(for_state)->get_var();
                    StateMent * data_state_gather = LetStat::make( data_simd_gather, Load::make( IncAddr::make( data_ptr_inc_var , inc_i ),true ) ); 

                    StateMent * index_state_gather = LetStat::make( index_simd_gather, Load::make( IncAddr::make( column_ptr_inc_var, inc_i ),true ) ); 
            
                    StateMent * x_state_gather = LetStat::make( x_simd_gather , Gather::make( x_ptr_v_var_, index_simd_gather , true_vec_const_ )  ); 

            
                    StateMent * fmadd_state_gather = LetStat::make(fast_res,  Add::make(fast_res, Mul::make( data_simd_gather , x_simd_gather ) ));  
                
                    std::vector<StateMent*> for_state_vec = {  data_state_gather, index_state_gather, x_state_gather, fmadd_state_gather };
                    StateMent * inner_for_state = CombinStatVec( for_state_vec );
                
                    dynamic_cast<For*>(for_state)->SetState(inner_for_state);
                     
                    state_vec.push_back( for_state );

                StateMent * store_state = Store::make( y_addr ,Add::make(y_data ,Reduce::make( fast_res )));
                state_vec.push_back(store_state); 
            

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
    load1_gather_addr_ptr_var_get_ = new Varience( __int64_ptr );
    std::string load1_gather_num_ptr_var_get_name = std::string("load1_gather_num_ptr");
    load1_gather_num_ptr_var_get_ = new Varience( __int_ptr, load1_gather_num_ptr_var_get_name );

    StateMent * column_ptr_state = LetStat::make( column_ptr_var_get_ , Load::make(IncAddr::make( column_ptr_ptr_var_ , mask_i_const)) );

    StateMent * data_ptr_state = LetStat::make( data_ptr_var_get_ , Load::make(IncAddr::make( data_ptr_ptr_var_ , mask_i_const)) );

    StateMent * row_ptr_state = LetStat::make( row_ptr_var_get_, Load::make(IncAddr::make( row_ptr_ptr_var_, mask_i_const )) );
    

    StateMent * column_v_ptr_state = LetStat::make( column_v_ptr_var_get_ , BitCast::make( column_ptr_var_get_ , __int_v_ptr)); 
    
    StateMent * row_v_ptr_state = LetStat::make( row_v_ptr_var_get_ , BitCast::make( row_ptr_var_get_ , __int_v_ptr));

    StateMent * data_v_ptr_state = LetStat::make( data_v_ptr_var_get_ , BitCast::make( data_ptr_var_get_ , __double_v_ptr)); 

    StateMent * load1_gather_addr_ptr_state = LetStat::make( load1_gather_addr_ptr_var_get_,  Load::make( IncAddr::make( load1_gather_addr_ptr_ptr_var_ , mask_i_const ) )  );

    StateMent * load1_gather_num_var_state = LetStat::make( load1_gather_num_ptr_var_get_,  Load::make( IncAddr::make( load1_gather_num_ptr_ptr_var_ , mask_i_const)  ));

    std::vector<StateMent*> state_vec_tmp = { column_ptr_state,data_ptr_state,row_ptr_state , column_v_ptr_state, data_v_ptr_state,row_v_ptr_state,load1_gather_addr_ptr_state, load1_gather_num_var_state};
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
    Varience * row_index = new Varience(__int);
    StateMent * init_state = GenerateInit(mask_i,circle_mask);
    
    Varience * load1_gather_num_ptr_var_get = new Varience( __int_ptr );

    Varience * load1_gather_num_var = new Varience( __int);
    std::string load1_gather_addr_ptr_var_name = std::string("load1_gather_addr_ptr_var");
    Varience * load1_gather_addr_ptr_var = new Varience( __int64_ptr,load1_gather_addr_ptr_var_name,false);

        StateMent * load1_gather_addr_ptr_state = LetStat::make(load1_gather_addr_ptr_var , load1_gather_addr_ptr_var_get_ ) ;
        init_state = Block::make( init_state, load1_gather_addr_ptr_state ); 
        Const * const_end = new Const( circle_num );
        StateMent * push_state = For::make(zero_const_ , one_const_, const_end);

        StateMent * inc_i = dynamic_cast<For*>(push_state)->get_var();

    StateMent * element_state;
        
        
        StateMent * row_index_state = LetStat::make( row_index , Load::make( IncAddr::make(row_ptr_var_get_, inc_i) )) ;

        StateMent * y_addr_state = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );

        StateMent * block_for_i_state = LetStat::make( block_for_i , Mul::make ( inc_i, new Const(inner_circle_num)) );

        StateMent * column_ptr_var_inc_state = LetStat::make( column_v_ptr_inc_var, IncAddr::make( column_v_ptr_var_get_, block_for_i) ) ;

        StateMent * data_ptr_var_inc_state = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, block_for_i) ) ;
        
        StateMent * load1_gather_num_var_get_state = LetStat::make( load1_gather_num_var , Load::make( IncAddr::make(load1_gather_num_ptr_var_get_ , inc_i )) );
        


        element_state = get_reduce_element( column_v_ptr_inc_var , data_v_ptr_inc_var , y_offset_ptr , mask , load1_gather_num_var, load1_gather_addr_ptr_var);

        StateMent * load1_gather_addr_ptr_var_state = LetStat::make(load1_gather_addr_ptr_var , IncAddr::make( load1_gather_addr_ptr_var, load1_gather_num_var ) );
         std::vector<StateMent*> inner_for_vec = { block_for_i_state, row_index_state,y_addr_state,column_ptr_var_inc_state,data_ptr_var_inc_state, load1_gather_num_var_get_state, element_state,load1_gather_addr_ptr_var_state  };
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
    
    Varience * load1_gather_num_var = new Varience( __int );
    StateMent * load1_gather_num_state = LetStat::make( load1_gather_num_var , Load::make( load1_gather_num_ptr_var_get_ ));
    state_vec_tmp.push_back(load1_gather_num_state);

    std::vector<StateMent *> load1_gather_num_for_state_inner_vec;

//    state_vec_tmp.push_back(  Print::make( load1_gather_num_var ));

    StateMent * load1_gather_num_for_state = For::make(zero_const_ , one_const_, load1_gather_num_var);


    state_vec_tmp.push_back(load1_gather_num_for_state); 
    StateMent * load1_gather_num_i = dynamic_cast<For*>(load1_gather_num_for_state)->get_var();

    Varience * y_offset_ptr = new Varience( __double_ptr);    
    column_ptr_inc_var = new Varience( __int_ptr ); 
    data_v_ptr_inc_var = new Varience( __double_v_ptr );
    
    Varience * row_index = new Varience( __int); 
    int load_gather_level = 1;
    
    StateMent * column_ptr_var_inc_state_spec = LetStat::make( column_ptr_inc_var, IncAddr::make( column_ptr_var_get_, load1_gather_num_i) ) ;
    
    StateMent * data_ptr_var_inc_state_spec = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, load1_gather_num_i) ) ;
        
    StateMent * row_index_state_spec = LetStat::make( row_index , Load::make( IncAddr::make(row_ptr_var_get_ , load1_gather_num_i ) )) ;

        
    StateMent * y_addr_state_spec = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index ) );
    
    Varience * load1_gather_addr = new Varience( __int64 );

    StateMent * load1_gather_addr_state = LetStat::make( load1_gather_addr, Load::make( IncAddr::make(load1_gather_addr_ptr_var_get_,load1_gather_num_i)));
        
        StateMent * element_state_spec = get_shuffle_element( column_ptr_inc_var , data_v_ptr_inc_var , y_offset_ptr , mask , trans_addr_, load_gather_level, load1_gather_addr) ;

        load1_gather_num_for_state_inner_vec.push_back( column_ptr_var_inc_state_spec );
        load1_gather_num_for_state_inner_vec.push_back( data_ptr_var_inc_state_spec );
        load1_gather_num_for_state_inner_vec.push_back( row_index_state_spec );

        load1_gather_num_for_state_inner_vec.push_back( y_addr_state_spec );

        load1_gather_num_for_state_inner_vec.push_back( load1_gather_addr_state );


        load1_gather_num_for_state_inner_vec.push_back( element_state_spec );
        dynamic_cast<For*>(load1_gather_num_for_state)->SetState(CombinStatVec(load1_gather_num_for_state_inner_vec));
/////////////////////////////////////////////////////////////////////
        y_offset_ptr = new Varience( __double_ptr);    
        column_ptr_inc_var = new Varience( __int_v_ptr ); 
        data_v_ptr_inc_var = new Varience( __double_v_ptr );
        Varience * row_index_gather = new Varience( __int);
                
        StateMent * gather_for_state = For::make(  load1_gather_num_var  , one_const_, new Const(circle_num));

        state_vec_tmp.push_back(gather_for_state);

        std::vector<StateMent*> gather_for_state_vec; 
        StateMent * begin_i_const = dynamic_cast<For*>( gather_for_state )->get_var();

//        Varience * begin_i_const = new Varience(__int);
//        StateMent * begin_i_const_state = LetStat::make( begin_i_const, Add::make( gather_for_i, load1_gather_num_var ) );
        StateMent * column_ptr_var_inc_state_gather = LetStat::make( column_v_ptr_inc_var, IncAddr::make( column_v_ptr_var_get_, begin_i_const) ) ;
        StateMent * data_ptr_var_inc_state_gather = LetStat::make( data_v_ptr_inc_var, IncAddr::make( data_v_ptr_var_get_, begin_i_const) ) ;
        
        StateMent * row_index_state_gather = LetStat::make( row_index_gather , Load::make( IncAddr::make(row_ptr_var_get_ ,begin_i_const) )) ;
        
        StateMent * y_addr_state_gather = LetStat::make( y_offset_ptr , IncAddr::make( y_ptr_var_, row_index_gather ) );

        StateMent * element_state_gather = get_shuffle_element( column_v_ptr_inc_var , data_v_ptr_inc_var , y_offset_ptr , mask , trans_addr_) ;

//        gather_for_state_vec.push_back( begin_i_const_state );
        gather_for_state_vec.push_back( column_ptr_var_inc_state_gather );
        gather_for_state_vec.push_back( data_ptr_var_inc_state_gather );
        gather_for_state_vec.push_back( row_index_state_gather );
        gather_for_state_vec.push_back( y_addr_state_gather );
        gather_for_state_vec.push_back( element_state_gather );
        dynamic_cast<For*>(gather_for_state)->SetState(CombinStatVec(gather_for_state_vec));

    return CombinStatVec(state_vec_tmp);    
}

