#ifndef STATEMENT_PRINT_HPP
#define STATEMENT_PRINT_HPP
#include "ir_func.hpp"
#include "type.hpp"
#include <iostream>
#define SET_DISPATCH(CLASS_NAME) SET_DISPATCH_TEMPLATE( ftype_ptr,CLASS_NAME,StateMent,print_ )
class StateMentPrint{
using FType = ir_func<void(StateMent*,std::ostream&)>; 
void print_(StateMent * stat,std::ostream&os) {
    printf("\nthe statement %s does not support\n", stat->get_class_name().c_str());
}
void print_(Block * stat,std::ostream&os) {
    print(stat->get_stat1(),os);
    print(stat->get_stat2(),os);
}
void print_(For * stat,std::ostream&os) {
    os << "for ( ";
    print(stat->get_var(),os);
    os << " = ";
    print(stat->get_begin(),os);
    os << " , ";
    print(stat->get_end(),os);
    os << " , ";
    print(stat->get_space(),os);
    os << " ) { \n";
    print(stat->get_stat(),os);
    os << "}\n";

}
void print_(Varience * stat , std::ostream&os) {
    os << stat->get_type_str() << " " << stat->get_name(); 
}
void print_(Const * stat, std::ostream&os) {
    os << stat->get_type_str() << "(" << stat->get_data_str() <<")";
}
void print_(LetStat * stat,std::ostream&os) {
    if(stat->get_is_const()) {
        os << "const ";
    }
    print_(stat->get_res(),os);
    os << " = ";
    print(stat->get_expr(),os);
    os << "\n";
}
void print_( IncAddr * stat ,std::ostream&os) {
    print(stat->get_addr(),os) ;
    os << " + ";
    print(stat->get_inc(),os);
    os << "\n";
}
void print_(Nop * stat,std::ostream&os ) {
    LOG(INFO) << "????";
    os << "nop\n";
}
void print_(Scatter * stat, std::ostream&os) {
    os << "scatter ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    os << ")";
    os << " <- ";
    print(stat->get_data(),os);
    os << "\n";

}
void print_(Init * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print(stat->get_data(),os);
    os << "\n";
}
void print_(Gather * stat, std::ostream&os ) {
    os << "gather ";
    print(stat->get_addr(),os);
    os << "(";
    print(stat->get_index(),os );
    os << ",";
    print(stat->get_mask(),os);
    os << ")\n";
}
void print_(Load * stat, std::ostream&os ) {
    os << "load ";
    os << "(";
    print(stat->get_addr(),os);
    os << ")";
}
void print_( Print * stat, std::ostream & os ) {
    os << "print ";
    print_(stat->get_var(),os);
    os << "\n";
}
void print_(Store * stat, std::ostream&os ) {
    os << stat->get_class_name() << " ";
    print(stat->get_addr(),os);
    os << " <- ";
    print(stat->get_data(),os);
}
void print_(Shuffle * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << " , ";
    print(stat->get_v2(),os);
    os << " , ";
    print(stat->get_index(),os);
    os << ")\n";
}
void print_(Reduce * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}
void print_(BroadCast * stat, std::ostream&os ) {
    os << stat->get_class_name() << " (";
    print(stat->get_v1(),os);
    os << ")\n";
}

void print_(BitCast * stat, std::ostream&os ) {

    os << stat->get_class_name() << " (";
    os << stat->get_type_str();
    os << ")";
    print( stat->get_v1(),os);
    os << "\n";
}
#define PRINT_BINARY( CLASSNAME ,OP) \
    void print_(CLASSNAME * stat,std::ostream& os ) {\
        print(stat->get_v1(),os); \
        os << " " OP " " ;\
        print(stat->get_v2(),os);\
    }

PRINT_BINARY( Binary,"op" )
PRINT_BINARY( Add, "+");

PRINT_BINARY( Div, "/");
PRINT_BINARY( Mul, "*");
public:
void print(StateMent * stat,std::ostream&os) {
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
    }
    (*ftype_ptr)(stat,os);
/*    printf("For\n");
    printf("the statement %s does not support\n", stat->get_class_name().c_str());
    print(stat->get_begin());
    print(stat->get_space());
    print(stat->get_num());
    print(stat->get_stat());*/ 
}
};
std::ostream& operator << (std::ostream &stream ,  StateMent*statement ) {
    StateMentPrint statement_print;
    statement_print.print( statement,stream );
    return stream;
}
#undef SET_DISPATCH
#endif
