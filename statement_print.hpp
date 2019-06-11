#ifndef STATEMENT_PRINT_HPP
#define STATEMENT_PRINT_HPP
#include "statement.hpp"
#include "ir_func.hpp"
#include "type.hpp"
#include <iostream>
class StateMentPrint{

void print_(StateMent * stat,std::ostream&os) ;

void print_(Block * stat,std::ostream&os) ;

void print_(For * stat,std::ostream&os) ;

void print_(Varience * stat , std::ostream&os) ;

void print_(Const * stat, std::ostream&os) ;

void print_(LetStat * stat,std::ostream&os) ;

void print_( IncAddr * stat ,std::ostream&os) ;

void print_(Nop * stat,std::ostream&os ) ;

void print_(Scatter * stat, std::ostream&os) ;

void print_(Init * stat, std::ostream&os ) ;

void print_(Gather * stat, std::ostream&os ) ;


void print_(Load * stat, std::ostream&os ) ;

void print_( Print * stat, std::ostream & os ) ;

void print_(Store * stat, std::ostream&os ) ;

void print_(Shuffle * stat, std::ostream&os ) ;

void print_(Reduce * stat, std::ostream&os ) ;

void print_(BroadCast * stat, std::ostream&os ) ;

void print_(BitCast * stat, std::ostream&os ) ;

void print_(Binary * stat, std::ostream&os ) ;

void print_(Add * stat, std::ostream&os );

void print_(Div * stat, std::ostream&os ) ;
void print_(Mul * stat, std::ostream&os ) ;

void print_(Minus * stat, std::ostream&os ) ;

void print_(DetectConflict * stat, std::ostream&os ) ;
void print_(ComplexReduce * stat, std::ostream&os ) ;
public:

void print(StateMent * stat,std::ostream&os) ;
};

std::ostream& operator << (std::ostream &stream ,  StateMent*statement ) ;
#endif
