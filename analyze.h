#ifndef ANALYZE_H
#define ANALYZE_H
#include "configure.hpp"
typedef struct ShuffleIndex {
    char index[VECTOR];
} ShuffleIndex, *ShuffleIndexPtr;

int analyze( int * vec ,ShuffleIndexPtr shuffle_index_ptr ) ;
#endif
