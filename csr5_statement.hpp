#ifndef CSR5_STATEMENT_HPP
#define CSR5_STATEMENT_HPP
#include "statement.hpp"
#include "statement_print.hpp"
//#include "llvm_lib/llvm_log.h"
//#include "llvm_lib/llvm_print.hpp"
#include "csr_matrix.h"
#include "bit2addr.h"
#include "analyze.h"
StateMent* CombinStatVec( const std::vector<StateMent*> &stat_vec ) ;


class PageRankStateMent : StateMent{
    ///const
    Const * zero_const_;
    Const * one_const_;
    Const * true_vec_const_; 
    Const * dzero_vec_const_; 
    Const * one_vec_const_;
    ///parameter
    Varience * y_ptr_var_;
    Varience * x_ptr_var_;
    Varience * data_ptr_var_;
    Varience * column_ptr_var_;
    Varience * row_ptr_var_;
    Varience * row_num_var_;
    Varience * row_ptr_ptr_var_;
    Varience * column_ptr_ptr_var_;
    Varience * data_ptr_ptr_var_;

    Varience * y_v_ptr_var_;
    Varience * x_v_ptr_var_;
    Varience * data_v_ptr_var_;
    Varience * column_v_ptr_var_;

    Varience * y_ptr_v_var_;
    Varience * x_ptr_v_var_;

    FuncStatement * func_state_ptr_;
    StateMent * init_state_;

	std::vector<StateMent*> state_vec_;
    ////middle patameter
    Varience * column_v_ptr_var_get_;
    Varience * data_v_ptr_var_get_;
    Varience * row_v_ptr_var_get_; 
   
    Varience * column_ptr_var_get_;
    Varience * data_ptr_var_get_;
    Varience * row_ptr_var_get_; 
    TransAddr trans_addr_;

    public:
    PageRankStateMent(  ) ;

    StateMent * get_block_element( Varience * column_v_ptr, Varience * data_v_ptr , Varience * row_v_ptr ,const Mask2 & mask_para ) ;
    StateMent * GenerateInit(int mask_i,int circle_mask); 
    StateMent * GenerateReduceState(const Mask2 & mask, const int circle_num, const int mask_i) ;
    StateMent * GenerateMaskState( const Mask2 & mask , const int circle_num , const int mask_i ) ;
//    StateMent * get_element(StateMent * index , Varience * y_addr, const Mask2& mask_para) ;

    StateMent * get_element(Varience * column_ptr_inc_var, Varience * data_ptr_inc_var , Varience * y_addr ,const Mask2 &mask_para,const TransAddr& trans_addr) ;
    void make(const std::map<Mask2,int>&mask_num_map );

    FuncStatement * get_function() ;
};
#endif

