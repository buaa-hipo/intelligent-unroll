#ifndef ANALYZE_H
#define ANALYZE_H
#include "configure.hpp"
typedef struct ShuffleIndex {
    char index[VECTOR];
} ShuffleIndex, *ShuffleIndexPtr;

int** Analyze( ShuffleIndexPtr shuffle_index_ptr , int * mask_vec,int nedges_pack_num, const int * n2 ,int * addr_num) ;
#endif
