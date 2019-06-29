#ifndef CSR5_STATEMENT_HPP
#define CSR5_STATEMENT_HPP
#include "statement.hpp"
#include "statement_print.hpp"
//#include "llvm_lib/llvm_log.h"
//#include "llvm_lib/llvm_print.hpp"
#include "csr_matrix.h"

StateMent* CombinStatVec( const std::vector<StateMent*> &stat_vec ) ;

    int generate_mask( int * index ) ;

class PageRankStateMent : StateMent{
    Varience * sum_var_;
    Varience * n1_var_;
    Varience * n2_var_;
    Varience * rank_var_;
    Varience * nneibor_var_;
    
    Varience * shuffle_index_var_;
    
    Varience * mask_addr_;

    Varience * sum_var_v_;
    Varience * n1_var_v_;
    Varience * n2_var_v_;
    Varience * rank_var_v_;
    Varience * nneibor_var_v_;
    
    Varience * shuffle_index_var_v_;
    Varience * nneibor_ptr_v_;
    Varience * sum_ptr_v_;
    Varience * rank_ptr_v_;
    FuncStatement * func_state_ptr_;
    StateMent * init_state_;

	std::vector<StateMent*> state_vec_;
    public:
    PageRankStateMent(  ) ;

    StateMent * get_element(StateMent * index ,int mask) ;
    void make( int * shuffle_num_vec,int mask_num,int * addr_num );

    FuncStatement * get_function() ;
};
#endif

