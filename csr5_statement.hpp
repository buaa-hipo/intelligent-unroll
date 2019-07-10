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

    Varience * y_ptr_var_;
    Varience * x_ptr_var_;
    Varience * data_ptr_var_;
    Varience * column_ptr_var_;
    Varience * row_ptr_var_;
    Varience * row_num_var_;
    Varience * row_ptr_ptr_var_;
    Varience * index_ptr_ptr_var_;

    Varience * y_v_ptr_var_;
    Varience * x_v_ptr_var_;
    Varience * data_v_ptr_var_;
    Varience * column_v_ptr_var_;

    Varience * y_ptr_v_var_;
    Varience * x_ptr_v_var_;

    FuncStatement * func_state_ptr_;
    StateMent * init_state_;

	std::vector<StateMent*> state_vec_;
    public:
    PageRankStateMent(  ) ;

    StateMent * get_element(StateMent * index , Varience * y_addr, const Mask2& mask_para) ;
    void make(const std::map<Mask2,int>&mask_num_map );

    FuncStatement * get_function() ;
};
#endif

