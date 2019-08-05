#ifndef STATEMENT_PASS_HPP
#define STATEMENT_PASS_HPP
#include "statement.hpp"
#include "ir_func.hpp"
#include "type.hpp"
#include <iostream>
class StateMentPass{
    StateMent* pass_(StateMent * stat ) ;
    StateMent* pass_(Block * stat ) ;
    StateMent* pass_(For * stat ) ;
    StateMent* pass_(Varience * stat   ) ;
    StateMent* pass_(Const * stat  ) ;
    StateMent* pass_(LetStat * stat ) ;
    StateMent* pass_( IncAddr * stat  ) ;
    StateMent* pass_(Nop * stat  ) ;
    StateMent* pass_(Scatter * stat  ) ;
    StateMent* pass_(Init * stat   ) ;
    StateMent* pass_(Gather * stat   ) ;
    StateMent* pass_(Load * stat  ) ;
    StateMent* pass_( Print * stat) ;
    StateMent* pass_(Store * stat  ) ;
    StateMent* pass_(Shuffle * stat   ) ;
    StateMent* pass_(Reduce * stat   ) ;
    StateMent* pass_(BroadCast * stat   ) ;
    StateMent* pass_(BitCast * stat   ) ;
    StateMent* pass_(Binary * stat   ) ;
    StateMent* pass_(Add * stat   );
    StateMent* pass_(Div * stat   ) ;
    StateMent* pass_(Minus * stat   ) ;
    StateMent* pass_(DetectConflict * stat ) ;
    StateMent* pass_(ComplexReduce * stat  ) ;
    StateMent* pass_( ExtractElement * stat);
    StateMent* pass_( InsertElement * stat) ;
public:

    StateMent* pass(StateMent * stat ) ;
};
#endif
