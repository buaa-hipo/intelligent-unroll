#include "csr_matrix.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <memory>

#include "immintrin.h"
PageRankStructurePtr test_read(std::string file_name ) {
    PageRankStructurePtr page_rank_structure_ptr = (PageRankStructurePtr)malloc(sizeof(PageRankStructure));
    int nnodes;
    int nedges;
    std::ifstream fin( file_name);
    fin >> nnodes >> nedges;
    int *n1 = (int *)_mm_malloc(sizeof(int)*nedges, 64);
    int *n2 = (int *)_mm_malloc(sizeof(int)*nedges, 64);
    int *nneibor = (int *)_mm_malloc(sizeof(int)*nnodes, 64);

    for(int i=0;i<nnodes;i++) {
        nneibor[i] = 0;
    }


    int t = 0;

    int w;
    while(fin >> n1[t] >> n2[t] >> w) {
        nneibor[n1[t]] += 1;
        t++;
    }

    float *rank = (float *)_mm_malloc(sizeof(float)*nnodes, 64);
    float *sum = (float *)_mm_malloc(sizeof(float)*nnodes, 64);

    for(int i=0;i<nnodes;i++) {
	    rank[i] = 1.0;
	    sum[i] = 0.0;
    }

    page_rank_structure_ptr->nedges = nedges;
    page_rank_structure_ptr->nnodes = nnodes; 

    page_rank_structure_ptr->n1 = n1;
    page_rank_structure_ptr->n2 = n2;
    page_rank_structure_ptr->nneibor = nneibor;
    page_rank_structure_ptr->sum = sum;
    page_rank_structure_ptr->rank = rank;
     
    return page_rank_structure_ptr;
}
