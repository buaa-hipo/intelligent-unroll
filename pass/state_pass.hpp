#ifndef STATEMENT_PASS_HPP
#define STATEMENT_PASS_HPP
#include "statement.hpp"
#include "ir_func.hpp"
#include "type.hpp"
#include <iostream>
class StateMentPass{
virtual    StateMent* pass_(StateMent * stat ) ;
virtual    StateMent* pass_(Block * stat ) ;
virtual    StateMent* pass_(For * stat ) ;
virtual    StateMent* pass_(Varience * stat   ) ;
virtual    StateMent* pass_(Const * stat  ) ;
virtual    StateMent* pass_(LetStat * stat ) ;
virtual    StateMent* pass_( IncAddr * stat  ) ;
virtual    StateMent* pass_(Nop * stat  ) ;
virtual    StateMent* pass_(Scatter * stat  ) ;
virtual    StateMent* pass_(Init * stat   ) ;
virtual    StateMent* pass_(Gather * stat   ) ;
virtual    StateMent* pass_(Load * stat  ) ;
virtual    StateMent* pass_( Print * stat) ;
virtual    StateMent* pass_(Store * stat  ) ;
virtual    StateMent* pass_(Shuffle * stat   ) ;
virtual    StateMent* pass_(Reduce * stat   ) ;
virtual    StateMent* pass_(BroadCast * stat   ) ;
virtual    StateMent* pass_(BitCast * stat   ) ;
virtual    StateMent* pass_(Binary * stat   ) ;
virtual    StateMent* pass_(Add * stat   );
virtual    StateMent* pass_(Div * stat   ) ;

virtual    StateMent* pass_(Mul * stat   ) ;
virtual    StateMent* pass_(Minus * stat   ) ;
virtual    StateMent* pass_(DetectConflict * stat ) ;
virtual    StateMent* pass_(ComplexReduce * stat  ) ;
virtual    StateMent* pass_( ExtractElement * stat);
virtual    StateMent* pass_( InsertElement * stat) ;
public:

    StateMent* pass(StateMent * stat ) ;
};
#endif
