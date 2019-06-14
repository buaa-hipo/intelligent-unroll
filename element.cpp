#include "csr5_statement.hpp"
StateMent * PageRankStateMent::get_element(StateMent * index ,int mask) {
        bool true_vec[VECTOR] ;
        for( int i = 0 ; i < VECTOR ; i++ ) {
	 	    true_vec[i] = true;
	    }
        Const * true_vec_const = new Const ( true_vec, VECTOR );
        Const * Four = new Const(4);

            Varience * nx_simd = new Varience( __int_v );
            Varience * ny_simd = new Varience( __int_v );
            Varience * rank_simd = new Varience( __float_v );
            Varience * nneibor_simd = new Varience( __int_v );
            Varience * sum_simd = new Varience( __float_v );

            Varience * div_simd = new Varience( __float_v );
            Varience * sum_simd_new = new Varience( __float_v );

            Varience * complex_reduce_simd = new Varience( __float_v );
            Varience * detect_simd = new Varience( __bool_v );

            StateMent * nx_state = LetStat::make( nx_simd, Load::make( IncAddr::make( n1_var_v_, index ) ) ); 

            StateMent * ny_state = LetStat::make( ny_simd, Load::make( IncAddr::make( n2_var_v_, index ) ) ); 
            
            StateMent * rank_state = LetStat::make( rank_simd , Gather::make( rank_ptr_v_, nx_simd , true_vec_const )  ); 

            StateMent * nneibor_state = LetStat::make( nneibor_simd , Gather::make( nneibor_ptr_v_, nx_simd , true_vec_const )  ); 
            StateMent * sum_state = LetStat::make( sum_simd , Gather::make( sum_ptr_v_ , ny_simd ,true_vec_const ) );

            StateMent * div_state = LetStat::make(div_simd,  Div::make( rank_simd, BitCast::make(nneibor_simd, __float_v )));  
            //printf("%x\nmark_grep",mask);
            StateMent * complex_reduce_state;
            StateMent * detect_state ;
            StateMent * calc_state;
            if( mask != 0x0 ) {
                complex_reduce_state = LetStat::make(complex_reduce_simd, ComplexReduce::make( div_simd ,ny_simd,IncAddr::make(shuffle_index_var_v_, Mul::make(index,Four) ),mask ));

                //calc_state = LetStat::make( sum_simd_new, Add::make( sum_simd, div_simd) );
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
            if( mask!=0 ) {
                
                block_state = Block::make( block_state, complex_reduce_state );

                block_state = Block::make( block_state, detect_state );
            }

            block_state = Block::make( block_state, calc_state );

            block_state = Block::make( block_state, scatter_state );
}
