#ifndef ANALYZE_H
#define ANALYZE_H
#include "configure.hpp"
#include <iostream>
#include <map>
#include <vector>
class Mask2{
    public:
    int num_;
    int mask_;
    Mask2() {
    }
    Mask2( int num , int mask ) : num_(num),mask_(mask) {
    
    } 
    bool can_block()const {
    //    if( mask_ != 0x1 || (mask_==0x1 && num_==1) )
        if( mask_ != 0x1 )
            return true;
        else
            return false;
    }
    inline bool operator<( const Mask2 & arg ) const {
        if( this->num_ < arg.num_ ) {
            return true;
        } else {
            if( this->num_ == arg.num_ && this->mask_ < arg.mask_ ) {
                return true;
            } else {
                return false;
            }
        }
    }

    inline bool operator==( const Mask2 & arg ) const {
        return arg.num_ == this->num_ && arg.mask_ == this->mask_;
    }
    friend std::ostream& operator<<( std::ostream & stream,const Mask2& mask ) {

        stream << std::hex << " mask 0x" << mask.mask_ ;
        stream << std::dec << " num :" << mask.num_ ;
        return stream;
    }
};

void Analyze( std::map< Mask2,std::pair<std::vector<int>,std::vector<int>>> &mask_map, std::map<Mask2,int> &mask_num_map, int & mask_num , int * row_ptr , int * column_ptr, const int row_num, const int column_num) ;

void Show( Mask2 * mask_ptr, const int mask_num ) ;
#endif
