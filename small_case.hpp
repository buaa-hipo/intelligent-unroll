#ifndef SMALL_CASE_HPP
#define SMALL_CASE_HPP
//dense
#include "util.h"
PageRankStructure little_test2( ) {
    #define NEDGES 16
    #define NNODES 16
    PageRankStructure page_rank_structure;
    const int nedges = NEDGES;
    const int nnodes = NNODES;
    int * n1 = SIMPLE_MALLOC(int,nedges);
    int * n2 = SIMPLE_MALLOC(int,nedges);

    int * nneibor = SIMPLE_MALLOC( int, nnodes );
    float * sum = SIMPLE_MALLOC( float , nnodes );
    
    float * rank = SIMPLE_MALLOC( float, nnodes );
    page_rank_structure.n1 = n1;
    page_rank_structure.n2 = n2;
    page_rank_structure.nneibor = nneibor;
    page_rank_structure.nedges = nedges;
    page_rank_structure.nnodes = nnodes;
    page_rank_structure.rank = rank;
    for( int i = 0 ; i < nedges ; i++) {
        n1[i] = i;
        n2[i] = i;
    } 
    for( int i = 0 ; i < nnodes ; i++ ) {
        nneibor[i] = i;
        rank[i] = i;
        sum[i] = 0;
    }

    #undef NEDGES
    #undef NNODES
    return page_rank_structure;
}

#endif
