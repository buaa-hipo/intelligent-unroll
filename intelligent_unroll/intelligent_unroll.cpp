#ifndef INTELLIGENT_UNROLL_HPP
#define INTELLIGENT_UNROLL_HPP
#include "llvm_lib/llvm_module.h"
#include "llvm_lib/llvm_codegen.hpp"
#include <vector>
#include <string>
#include <stdarg.h>
#include "transform_data.hpp"
class IntelligentUnroll{

    ///produced by parse expression

    std::set<std::string> gather_set_;
    std::set<std::string> scatter_set_;
    std::set<std::string> reduction_set_;
    std::map<std::string, VarType >  name_type_map_,

    std::vector<std::string> input_var_vec_;
    std::set<std::string> iterates_set_;

    Node * root_node_ptr;
    /////produced by generate information    
    std::map<std::string,GatherInfo*> gather_map_;
    std::map<std::string,ScatterInfo*> scatter_map_;
    std::map<std::string,ReductionInfo*> reduction_map_;
    std::unordered_map<size_t,std::vector<int>> same_feature_map_;
    std::unordered_map<size_t,std::vector<std::pair<int,int>>>same_feature_range_map_;

    /////produced by transform_data
    std::map<std::string,int*>  gather_name_new_ptr_map_;
    std::map<std::string,int*>  reduction_name_new_ptr_map_;
    std::map<std::string,int*>  scatter_name_new_ptr_map_;
    std::map<std::string,void*>  name_new_ptr_map_;
    ///produced by code seed generate
    std::map<std::string, Varience*>  gather_name_new_ptr_map_;
    std::map<std::string, Varience*>  reduction_name_new_ptr_map_;
    std::map<std::string, Varience*>  scatter_name_new_ptr_map_;
    std::map<std::string, Varience*>  name_new_ptr_map_;
    ////produced by passes
    StateMent * calculate_state_; 
    //
    IntelligentUnroll(  ) {
    }
    //////////////////
    uint64_t compiler( const std::string & expr_str,  std::map<std::string,void*> & name2ptr_map  ,int table_column_num) {

        ////////////parse the expression, get a node tree
        parse_expression( expr_str,
                          root_node_ptr_,
                          gather_set_,
                          scatter_set_,
                          reduction_set_,
                          name_type_map_,
                          input_var_vec_,
                          iterates_set_
                          );
        ///////////generate information
        generate_information( scatter_set_, reduction_set_, gather_set_, name2ptr_map,table_column_num,gather_map_,scatter_map_,reduction_map_,same_feature_range_map_,same_feature_range_map_  );
        ///////////transform data
        transform_data( transform_data_set_, name_type_map_,
                        name_ptr_map_,
                        gather_set_,
                        scatter_set_,
                        reduction_set_,
                        same_feature_map_,
                        same_feature_range_map_,
                        gather_name_new_ptr_map_,
                        reduction_name_new_ptr_map_,
                        scatter_name_new_ptr_map_,
                        name_new_ptr_map_
        ); 

        ////////////////node tree to code seed
        Node2StateMent * node2statement_ptr = new Node2StateMent( node_ptr );
        FuncStatement * func_statement_ptr = node2statement_ptr->generate_func();
        StateMent * code_seed_statement_ptr = node2statement_ptr->generate_code_seed();
        ///////////// process code_seed_statement
        ....
        /////////////combin statement with func_statement
        ....
        /////////// 
        LLVMCodeGen codegen;
        codegen.AddFunction( func_statement_ptr );
        llvm_module_ptr_ = new LLVMModule< FunctionType >( codegen.get_mod(),codegen.get_ctx() );

        llvm_module_ptr_->Init("llvm -mcpu=knl  -mattr=+avx512f,+avx512pf,+avx512er,+avx512cd,+fma,+avx2,+fxsr,+mmx,+sse,+sse2,+x87,+fma,+avx2,+avx");
         
        uint64_t func = llvm_module_ptr_->GetFunction("function");
        return func;
    }
} 

#endif
