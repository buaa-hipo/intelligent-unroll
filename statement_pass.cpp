#include "statement_pass.hpp"

#define SET_DISPATCH(CLASS_NAME) SET_DISPATCH_TEMPLATE( ftype_ptr,CLASS_NAME,StateMent,pass_ )
StateMent* StateMentPass::pass_(StateMent * stat) {
    LOG(FATAL) << "\nthe statement %s does not support\n" << stat->get_class_name() ;
    return nullptr;
}
StateMent* StateMentPass::pass_(Block * stat) {
    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    std::vector<StateMent* > * new_state_vec = new std::vector<StateMent*>() ;
    new_state_vec->resize( state_vec.size() , nullptr);
    bool is_change = false;
    for(int i = 0 ; i < state_vec->size() ; i++) {
        StateMent * new_state = pass((*state_vec)[i]);
        if( new_state != (*state_vec)[i] ) {
            is_change = true;
        }

        new_state_vec[i] = new_state; 
    }
    if(is_change) {
        state_vec->clear();
        delete * state_vec;

        return Block::make( new_state_vec ); 
    } else {
        new_state_vec->clear();
        delete *new_state_vec;
        return stat;
    }
}
StateMent* StateMentPass::pass_(For * stat) {
//    StateMent * var_state = stat->get_var();
    StateMent * begin_state = stat->get_begin();
    StateMent * end_state = stat->get_end();
    StateMent * space_state = stat->get_space();
    StateMent * state_state = stat->Get_stat();


//    StateMent * var_state_new = pass(var_state);
    StateMent * begin_state_new = pass(begin_state);
    StateMent * end_state_new = pass(end_state);
    StateMent * space_state_new = pass(space_state);

    StateMent * state_state_new = pass(state_state);
//    if( var_state == var_state_new &&
  if(      begin_state == begin_state_new &&
        end_state == end_state_new &&
        space_state == space_state_new &&
        state_state == state_state_new) {
        return stat;
    } else {
        return For::make(begin_state_new,space_state_new,end_state_new,state_state_new); 
    }
    
}
StateMent* StateMentPass::pass_(Varience * stat) {
    //os << stat->get_type_str() << " " << stat->get_name(); 
    return stat;
}
StateMent* StateMentPass::pass_(Const * stat) {
//    os << stat->get_type_str() << "(" << stat->get_data_str() <<")";
    return stat;
}
StateMent* StateMentPass::pass_(LetStat * stat) {
    
    Varience * res_var = stat->get_res();
    StateMent * expr_state = stat->get_expt();

    Varience * res_var_new = pass_(stat->get_res());
    
    StateMent * expr_state_new = pass( expr_state);
    if( res_var_new == res_var && expr_state == expr_state_new ){
        return stat;
    } else { 
        return LetStat::make( res_var_new,expr_state_new );
    }
}
StateMent* StateMentPass::pass_( IncAddr * stat ) {
    print(stat->get_addr(),os) ;
    os << " + ";
    print(stat->get_inc(),os);
}
void StateMentPrint::print_(Nop * stat,std::ostream&os ) {
    os << "nop\n";
}
void StateMentPrint::print_(Scatter * stat, std::ostream&os) {
    PrintSpace(os);
    os << "scatter ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    os << ",";

    print(stat->get_mask(),os );
    os << ")";
    os << " <- ";
    print(stat->get_data(),os);
    os << "\n";

}
void StateMentPrint::print_(Init * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print(stat->get_data(),os);
    os << "\n";
}
void StateMentPrint::print_(Gather * stat, std::ostream&os ) {
    os << "gather ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    os << ",";
    print(stat->get_mask(),os);
    os << ")\n";
}
void StateMentPrint::print_(Load * stat, std::ostream&os ) {
    os << "load ";
    os << "(";
    print(stat->get_addr(),os);
    os << ")";
}
void StateMentPrint::print_( Print * stat, std::ostream & os ) {
    os << "print ";
    print(stat->get_var(),os);
    os << "\n";
}
void StateMentPrint::print_( ExtractElement * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print(stat->get_from(),os);
    os << " [ ";
    print(stat->get_index(),os);
    os << "]";
}
void StateMentPrint::print_( InsertElement * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print( stat->get_to(),os );
    os << "[";
    print(stat->get_index(),os);
    os << "] <- ";
    print(stat->get_from(),os);
    os << "\n";
}

void StateMentPrint::print_(Store * stat, std::ostream&os ) {
    PrintSpace(os);
    os << stat->get_class_name() << " ";
    print(stat->get_addr(),os);
    os << " <- ";
    print(stat->get_data(),os);
    os << "\n";
}
void StateMentPrint::print_(Shuffle * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << " , ";
    if(stat->get_v2() != NULL) {
        print(stat->get_v2(),os);
        os << " , ";
    }
    print(stat->get_index(),os);
    os << ")\n";
}
void StateMentPrint::print_(ComplexReduce * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os); 
    os << ",";
    print(stat->get_index(),os);
    os << ",";
    os << stat->get_mask();
    os << ")\n";
}
void StateMentPrint::print_(DetectConflict * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_index(),os);
    os << ")\n";
}


void StateMentPrint::print_(Reduce * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}
void StateMentPrint::print_(BroadCast * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}

void StateMentPrint::print_(BitCast * stat, std::ostream&os ) {

    os << stat->get_class_name() << " (";
    os << stat->get_type_str();
    os << ")";
    print( stat->get_v1(),os);
}
#define PRINT_BINARY( CLASSNAME ,OP) \
    void StateMentPrint::print_(CLASSNAME * stat,std::ostream& os ) {\
        print(stat->get_v1(),os); \
        os << " " OP " " ;\
        print(stat->get_v2(),os);\
    }

PRINT_BINARY( Binary,"op" )
PRINT_BINARY( Add, "+");

PRINT_BINARY( Div, "/");
PRINT_BINARY( Mul, "*");

PRINT_BINARY( Minus, "-");

void StateMentPrint::pass(StateMent * stat) {

    using FType = ir_func<StateMent*(StateMent*)>; 
    static FType  * ftype_ptr = nullptr;
    if(ftype_ptr == nullptr) {
        ftype_ptr = new FType();
        SET_DISPATCH( StateMent );
        SET_DISPATCH( Block );
        SET_DISPATCH( For );
        SET_DISPATCH(Varience);

        SET_DISPATCH( Const );

        SET_DISPATCH( LetStat );
        SET_DISPATCH(IncAddr);
        SET_DISPATCH(Nop);
        SET_DISPATCH(Scatter);
        SET_DISPATCH(Init);
        SET_DISPATCH(Gather);
        SET_DISPATCH( Load );
        SET_DISPATCH(Store);
        SET_DISPATCH(Shuffle);
        SET_DISPATCH( Reduce );
        SET_DISPATCH(BitCast);
        SET_DISPATCH(Binary);
        SET_DISPATCH(Add);
        SET_DISPATCH(Mul);

        SET_DISPATCH(Div);
        SET_DISPATCH(Minus);
        SET_DISPATCH(Print);
        SET_DISPATCH(BroadCast);

        SET_DISPATCH(ComplexReduce);
        SET_DISPATCH(DetectConflict  );

        SET_DISPATCH(ExtractElement );
        SET_DISPATCH(InsertElement );
    }
    (*ftype_ptr)(stat,os);

}
