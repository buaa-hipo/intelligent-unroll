#ifndef CSR5_STATEMENT_HPP
#define CSR5_STATEMENT_HPP
#include "statement.hpp"
#include "statement_print.hpp"
#include "llvm_lib/llvm_log.h"
#include "llvm_lib/llvm_print.hpp"
class CSR5FastStateMent : StateMent{
    int omega_;
    int delta_;
//    Varience * data_addr_; 
//    Varience * index_addr_;
//    Varience * x_addr_ ;
    Varience * data4_addr_; 
    Varience * index4_addr_;
    Varience * x4_addr_ ;

    Varience * x_addr_v4_;
    public:
    CSR5FastStateMent(const int omega, const int delta,Varience * data4_addr,Varience * index4_addr,Varience * x4_addr,Varience * x_addr_v4) : omega_(omega),delta_(delta),data4_addr_(data4_addr),index4_addr_(index4_addr),x4_addr_(x4_addr),x_addr_v4_(x_addr_v4) { 
         
    };

    StateMent * make( Varience * res, StateMent * begin, StateMent * space, StateMent* num ) {
         Varience * index = new Varience(__int_vector_type) ;
         Varience * gather_data = new Varience(__double_vector_type);
         Varience * m_data = new Varience(__double_vector_type);
         bool true_vec[VECTOR4] = {true,true,true,true};
         Const * true_vec4 = new Const ( true_vec, VECTOR4 );
         Const * const_zero = new Const(0);
         StateMent * for_stat = For::make( const_zero , space,num  );
         StateMent * inc_i = dynamic_cast<For*>(for_stat)->get_var();
         StateMent * index_stat = LetStat::make( index, Load::make( IncAddr::make(index4_addr_, Add::make(inc_i,begin) )));

         StateMent * mdata_load_stat = LetStat::make( m_data, Load::make( IncAddr::make(data4_addr_, Add::make(inc_i,begin))));

         StateMent * gather_stat = LetStat::make(gather_data, Gather::make( x_addr_v4_,index, true_vec4 ));
        
         StateMent * res_stat = LetStat::make(res,Add::make( res, Mul::make( gather_data, m_data )),false);
         StateMent * block_stat = Block::make( index_stat,mdata_load_stat );
         block_stat = Block::make( block_stat, gather_stat );
         block_stat = Block::make( block_stat,res_stat );
         dynamic_cast<For*>(for_stat)->SetState(block_stat);
         return for_stat;
    }
};
class CSR5StateMent : StateMent{
    int omega_;
    int delta_;
    Varience * data_addr_;
    Varience * index_addr_;
    Varience * x_addr_;
    Varience * y_addr_;
    Varience * tile_row_addr_;
    Varience * data4_addr_; 
    Varience * index4_addr_;
    Varience * x4_addr_ ;
    Varience * x_addr_v4_;

    CSR5FastStateMent * csr5_fast_statement_ptr_;
    FuncStatement * func_state_ptr_;
    StateMent * init_state_;
    public:
    CSR5StateMent( const int omega, const int delta ) : omega_(omega),delta_(delta) {
//        data_addr_ = new Varience( data_ptr );
//        x_addr_ = new Varience(x_ptr );
//        y_addr_ = new Varience(y_ptr);
//        index_addr_ = new Varience(index_ptr);
        std::vector<Type> args_type = { __double_ptr,__double_ptr,__double_ptr,__int_ptr,__int_ptr };
        Type ret_type = __void; 
        func_state_ptr_ = new FuncStatement( __void, args_type );
        y_addr_ = (*func_state_ptr_->get_args())[0];

        x_addr_ = (*func_state_ptr_->get_args())[1];
        data_addr_ = (*func_state_ptr_->get_args())[2];
        index_addr_ = (*func_state_ptr_->get_args())[3];
        tile_row_addr_ = (*func_state_ptr_->get_args())[4];
        
        data4_addr_ = new Varience( __double_v4_pointer );
        index4_addr_ = new Varience( __int_v4_pointer);
        x4_addr_ = new Varience( __double_v4_pointer );
        
        x_addr_v4_ = new Varience( __double_pointer_v4 );

        init_state_ = LetStat::make( data4_addr_, BitCast::make( data_addr_, __double_v4_pointer ) );
         
        init_state_ = Block::make(init_state_, LetStat::make( x4_addr_, BitCast::make( x_addr_, __double_v4_pointer ) ));

        init_state_ = Block::make(init_state_,LetStat::make( index4_addr_, BitCast::make( index_addr_, __int_v4_pointer ) ));
        init_state_ = Block::make(init_state_,LetStat::make( x_addr_v4_, BroadCast::make( x_addr_) ));


        csr5_fast_statement_ptr_ = new CSR5FastStateMent( omega_, delta_, data4_addr_,index4_addr_,x4_addr_,x_addr_v4_);
    };
    StateMent * CombinStatVec( std::vector<StateMent*> stat_vec ) {
        if( stat_vec.size() == 0 ) { return Nop::make();}
        else if(stat_vec.size() == 1) {return stat_vec[0];}
        else {
            StateMent * block_stat = Block::make( stat_vec[0],stat_vec[1]);
            for( int i = 2 ; i < stat_vec.size(); i++ ) {
                block_stat = Block::make( block_stat, stat_vec[i] );
            }
            return block_stat;
        }
    }
    FuncStatement * make( std::vector<CSR5SuperBlock*>& CSR5SuperBlockVec )  {

        std::vector<StateMent*> state_vec;
        double zero_v4[4] = {0,0,0,0};
        Const * zero_v4_const = new Const(zero_v4,VECTOR4);
        Const * zero_const = new Const(0);
        int tile_i = 0;
        for( auto &it : CSR5SuperBlockVec ) {
            TileType * tile_type_ptr = it->getTileTypeOrder();
            int * num_ptr = it->get_num();

            for( int supertile_i = 0 ; supertile_i < TileTypeNum; supertile_i++ ) {
                const TileType tile_type = tile_type_ptr[supertile_i];
                const int num = num_ptr[supertile_i];
                Varience * fast_res = new  Varience( __double_v4 ); 
                StateMent * init = LetStat::make( fast_res , zero_v4_const,false);
                Const * delta_const = new Const(delta_);
                Const * tile_i_const = new Const( tile_i );
                Const * one_const = new Const(1);
                Const * num_const = new Const(num);
                Varience * outer_var;
                StateMent * for_stat;
                StateMent * fast_stat;
                Varience * outer_var_mult_delta = new Varience(__int);

                StateMent * outer_var_mult8_stat;
                StateMent * combin_stat;
                StateMent * y_addr_inc;
                Varience * y_addr_inc_var = new Varience(__int);
                switch(tile_type) {
                    case FAST:
                       
                       state_vec.push_back(init);
                       for_stat = For::make( tile_i_const, one_const, Add::make(num_const, tile_i_const) );
                       outer_var = dynamic_cast<For*>(for_stat)->get_var();
                       outer_var_mult8_stat = LetStat::make(outer_var_mult_delta, Mul::make( outer_var  , delta_const ));
                       fast_stat = csr5_fast_statement_ptr_->make( fast_res, outer_var_mult_delta , one_const, delta_const);
                       combin_stat = Block::make( outer_var_mult8_stat, fast_stat );

                       dynamic_cast<For*>(for_stat)->SetState(combin_stat);

                       state_vec.push_back(for_stat);
                       
                       y_addr_inc = Load::make( IncAddr::make( tile_row_addr_ , Minus::make(outer_var, one_const) ));
                       
                       state_vec.push_back(LetStat::make( y_addr_inc_var, y_addr_inc )) ;
                       state_vec.push_back(Store::make( IncAddr::make(y_addr_,y_addr_inc_var),Reduce::make( fast_res )));
                       break; 
                    case SWITCH:
                        break;
                    case STORE:
                        break;
                    case NOP:
                        break;
                    default:
                       break;
                }
                tile_i += num;
            }
        }

        StateMent * state = CombinStatVec(state_vec);
        state = Block::make(init_state_,state);

        func_state_ptr_->set_state(state);

        return func_state_ptr_;
    }
     
};

#endif

