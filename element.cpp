#include "csr5_statement.hpp"
#include "bit2addr.h"
StateMent * PageRankStateMent::get_element(StateMent * index , Varience * y_addr ,const Mask2 &mask_para) {
        const int circle_num = mask_para.num_;
        const int circle_mask = mask_para.mask_;
        Bit2Addr bit2addr(VECTOR);
        TransAddr trans_addr = bit2addr.generate( circle_mask );
        
        const int reduce_num = trans_addr.num;
        const char * reduce_addr = trans_addr.addr;

        Const * const_zero = new Const(0);

        Const * const_one = new Const(1);
        Const * const_end = new Const( circle_num );

        bool true_vec[VECTOR] ;
        double dzero_vec[VECTOR];
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
        
        for( int i = 0 ; i < VECTOR ; i++ ) {
	 	    true_vec[i] = true;
            dzero_vec[i] = 0;
	    }
        for( int i = 0 ; i < reduce_num ; i++ ) {
            for( int j = 0 ; j < VECTOR ; j++ ) {
                reduce_addr_int[ i* VECTOR + j ] = (int)reduce_addr[ i * VECTOR + j ];
            }
        }

        Const * true_vec_const = new Const ( true_vec, VECTOR );
        Const * dzero_vec_const = new Const( dzero_vec , VECTOR );

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

                StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make( data_v_ptr_var_, index ),true ) ); 

                StateMent * index_state = LetStat::make( index_simd, Load::make( IncAddr::make( column_v_ptr_var_, index ),true ) ); 
            
                StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const )  ); 


                StateMent * mul_state = LetStat::make(mul_simd,  Mul::make( data_simd , x_simd ),false );  
                
                state_vec.push_back( data_state );state_vec.push_back( index_state );
                state_vec.push_back(x_state); state_vec.push_back(mul_state);


                StateMent * complex_reduce_state;
                StateMent * detect_state ;
                StateMent * calc_state;

                for( int reduce_i = 0 ; reduce_i < reduce_num ; reduce_i++ ) {
                    StateMent * shuffle_state = LetStat::make( shuffle_simd, Shuffle::make( mul_simd,dzero_vec_const, shuffle_index_const_vec[reduce_i] ),false); 
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
                StateMent * compress_state = LetStat::make( compress_simd, Shuffle::make( mul_simd, dzero_vec_const, compress_const ) );
                 

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
                    StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make( data_v_ptr_var_, index ),true ) ); 

                    StateMent * index_state = LetStat::make( index_simd, Load::make( IncAddr::make( column_v_ptr_var_, index ),true ) ); 
            
                    StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const )  ); 

            
                    StateMent * fmadd_state = LetStat::make(fast_res, Mul::make( data_simd , x_simd ) ,false);  
                    state_vec.push_back( data_state );

                    state_vec.push_back( index_state );
                    state_vec.push_back( x_state );
                    state_vec.push_back( fmadd_state );
                } else {

                    StateMent * init_state = LetStat::make( fast_res , dzero_vec_const,false);
                    StateMent * for_state = For::make(const_zero , const_one, const_end);
                     
                    StateMent * inc_i = dynamic_cast<For*>(for_state)->get_var();
                    StateMent * new_index_state = LetStat::make( new_index ,Add::make( inc_i, index ));
                    StateMent * data_state = LetStat::make( data_simd, Load::make( IncAddr::make( data_v_ptr_var_, new_index ),true ) ); 

                    StateMent * index_state = LetStat::make( index_simd, Load::make( IncAddr::make( column_v_ptr_var_, new_index ),true ) ); 
            
                    StateMent * x_state = LetStat::make( x_simd , Gather::make( x_ptr_v_var_, index_simd , true_vec_const )  ); 

            
                    StateMent * fmadd_state = LetStat::make(fast_res,  Add::make(fast_res, Mul::make( data_simd , x_simd ) ),false);  
                
                    std::vector<StateMent*> for_state_vec = { new_index_state, data_state, index_state, x_state, fmadd_state };
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
