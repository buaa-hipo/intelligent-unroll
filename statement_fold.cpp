#include "statement_fold.hpp"
std::vector<StateMent*> * statement_fold(const std::vector<StateMent*> & state_vec, int * mask ) {
     const int size = state_vec.size();
     std::vector<StateMent*> * ret_state_vec_ptr = new std::vector<StateMent*>();
     if(size > 0) {
         int old_mask = mask[0];
         int num = 1 ;
         for( int i = 1 ; i < size ; i++ ) {
             if( old_mask != mask[i] ) {
                StateMent * push_state;
                if( num == 1 ) {
                    push_state = state_vec[i-1];
                } else {
                    StateMent * state = state_vec[i-1];

                    push_state = For::make(  )
                }
                (*ret_state_vec_ptr).push_back(push_state);
                num = 1;
             } else {
                num++;
             }
        }
                if( num == 1 ) {
                    push_state = state_vec[i-1];
                } else {
                    StateMent * state = state_vec[i-1];

                    push_state = For::make(  )
                }
                (*ret_state_vec_ptr).push_back(push_state);

     }
}
