#ifndef PARSE_HPP
#define PARSE_HPP
typedef enum{Input, Output,Float,Double,Int, Var, Lambda,LeftBracket, RightRracket, Colon, Mul, Add,MulEqual,Equal,AddEqual,Div, End,Comma } TokenType;
typedef struct Token {
    TokenType token_type_;
    std::string token_name_;
} Token; 
typedef enum{ Mult, Add, Div , Gather, Scatter,Load } OpType;
typedef enum{ var_float,var_double,var_int,var_float_ptr,var_double_ptr,var_int_ptr } VarType;
VarType GetBasicType( VarType var_type ) {
     switch( token_type  ) {
        case FloatPtr:
            return var_float;
        case DoublePtr:
            return var_double;
        case IntPtr:
            return var_int;
        default:
            LOG(FATAL) << "Transfer fault";
    }

}
VarType TokenType2VarType( TokenType token_type ) {

    switch( token_type  ) {
        case FloatPtr:
            return var_float_ptr;
        case DoublePtr:
            return var_double_ptr;
        case IntPtr:
            return var_int_ptr;
        case Float:
            return var_float;
        case Double:
            return var_double;
        case Int:
            return var_int;
        default:
            LOG(FATAL) << "Transfer fault";
    }
}
void parse_expression( 
        const std::string expr_str,
        ////output
        Node * &root_node_ptr,
        std::set<std::string> &gather_set,
        std::set<std::string> &scatter_set,
        std::set<std::string> &reduction_set,
        std::map<std::string, VarType >  &name_type_map,
        std::vector<std::string> & input_var_vec,
        std::set<std::string> & iterates_set
        ) ;

#endif
