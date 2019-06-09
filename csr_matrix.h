#ifndef CSR_MATRIX_H
#define CSR_MATRIX_H
typedef struct PageRankStructure {
    int nedges;
    int nnodes;
    int * n1;
    int * n2;
    int * nneibor;
    float * sum;
    float * rank;
}PageRankStructure , *PageRankStructurePtr;
#endif
