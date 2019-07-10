#ifndef BIT2ADDR_H
#define BIT2ADDR_H
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

typedef  struct TransAddr {
    int num;
    char * addr;
}TransAddr ,  *TransAddrPtr ;

class Bit2Addr{
    
    public:
    int vector_;
    TransAddr trans_addr_;
    Bit2Addr( int vector ) : vector_(vector) {}
    int get_max_len( const int mask ) ;
    friend std::ostream & operator << ( std::ostream & stream , const Bit2Addr& para ) ;
    TransAddr generate( int mask ) ;

};
#endif
