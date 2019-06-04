#ifndef SMALL_CASE_HPP
#define SMALL_CASE_HPP
//dense
#include "util.h"
csrSparseMatrix little_test2(int row_num, int column_num ) {
    csrSparseMatrix csr_sparse_matrix;
    int * column_array = (int*)malloc(sizeof(int)* row_num * column_num);
    for( int row_i = 0 ; row_i < row_num ; row_i++ ) {
        for( int c_i = 0 ; c_i < column_num ; c_i++ ) {
            column_array[c_i + row_i * column_num] = c_i;
        }
    }

    int * row_array = (int *)malloc(sizeof(int)*(row_num+1));
    for( int i = 0 ; i <= row_num ; i++) {
        row_array[i] = i * column_num;
    }
    DATATYPE * A_array = (DATATYPE*)malloc(sizeof(DATATYPE) * row_num * column_num) ;
    for( int i = 0 ; i < row_num * column_num; i++ ) {
#ifdef DEBUG
        A_array[i] = 1;
#else
        A_array[i] = i;
#endif
    }

    csr_sparse_matrix.row_num = row_num;
    csr_sparse_matrix.column_num = column_num;
    csr_sparse_matrix.data_num = row_array[row_num];
    const int data_num = csr_sparse_matrix.data_num;
    csr_sparse_matrix.row_ptr = (int*)_mm_malloc(sizeof(int) * (row_num+1), ANONYMOUSLIB_X86_CACHELINE );

    csr_sparse_matrix.column_ptr = (int*)_mm_malloc(sizeof(int) * (data_num),ANONYMOUSLIB_X86_CACHELINE);

    csr_sparse_matrix.data_ptr = (DATATYPE*)_mm_malloc(sizeof(DATATYPE) * (data_num),ANONYMOUSLIB_X86_CACHELINE);
    for( int i = 0 ; i < row_num + 1; i++ ) {
        csr_sparse_matrix.row_ptr[i] = row_array[i];
    }
    for( int i = 0 ; i < data_num ; i++ ) {
        csr_sparse_matrix.column_ptr[i] = column_array[i];

        csr_sparse_matrix.data_ptr[i] = A_array[i];
    }
    #undef ROW_NUM 
    #undef COLUMN_NUM
    return csr_sparse_matrix;
}


csrSparseMatrix little_test2( ) {
    csrSparseMatrix csr_sparse_matrix;
    #define ROW_NUM 1
    #define COLUMN_NUM 128
    const int row_num = ROW_NUM;
    const int column_num = COLUMN_NUM;
    int column_array[COLUMN_NUM];
    for( int i = 0 ; i < column_num ; i++ ) {
        column_array[i] = i;
    }
    int row_array[ROW_NUM+1] = { 0, COLUMN_NUM };
    
    DATATYPE A_array[row_array[row_num]];
    for( int i = 0 ; i < row_array[row_num]; i++ ) {
//#ifdef DEBUG
//        A_array[i] = 1;
//#else
        A_array[i] = i;
//#endif
    }

    csr_sparse_matrix.row_num = row_num;
    csr_sparse_matrix.column_num = column_num;
    csr_sparse_matrix.data_num = row_array[row_num];
    const int data_num = csr_sparse_matrix.data_num;
    csr_sparse_matrix.row_ptr = (int*)_mm_malloc(sizeof(int) * (row_num+1), ANONYMOUSLIB_X86_CACHELINE );

    csr_sparse_matrix.column_ptr = (int*)_mm_malloc(sizeof(int) * (data_num),ANONYMOUSLIB_X86_CACHELINE);

    csr_sparse_matrix.data_ptr = (DATATYPE*)_mm_malloc(sizeof(DATATYPE) * (data_num),ANONYMOUSLIB_X86_CACHELINE);
    for( int i = 0 ; i < row_num + 1; i++ ) {
        csr_sparse_matrix.row_ptr[i] = row_array[i];
    }
    for( int i = 0 ; i < data_num ; i++ ) {
        csr_sparse_matrix.column_ptr[i] = column_array[i];

        csr_sparse_matrix.data_ptr[i] = A_array[i];
    }
    #undef ROW_NUM 
    #undef COLUMN_NUM
    return csr_sparse_matrix;
}

csrSparseMatrix little_test( ) {
    csrSparseMatrix csr_sparse_matrix;
    #define ROW_NUM 9
    #define COLUMN_NUM 8
    const int row_num = ROW_NUM;
    const int column_num = COLUMN_NUM;
    int column_array[] = { 2,3,0,1, 2,3,4,5, 6,1,3,4, 5,1,3,4 ,
                           6,7,0,3, 6,0,3,4, 6,7,2,0,1,2, 3,4,6,7, 
                           };
    int row_array[ROW_NUM+1] = {0,2,9,9,  13,18,21,26,  27,34 };
    
    DATATYPE A_array[34];
    for( int i = 0 ; i < 34; i++ ) {
#ifdef DEBUG
        A_array[i] = 1;
#else
        A_array[i] = i;
#endif
    }

    csr_sparse_matrix.row_num = row_num;
    csr_sparse_matrix.column_num = column_num;
    csr_sparse_matrix.data_num = row_array[row_num];

    const int data_num = csr_sparse_matrix.data_num;
    csr_sparse_matrix.row_ptr = (int*)_mm_malloc(sizeof(int) * (row_num+1),ANONYMOUSLIB_X86_CACHELINE);

    csr_sparse_matrix.column_ptr = (int*)_mm_malloc(sizeof(int) * (data_num),ANONYMOUSLIB_X86_CACHELINE);

    csr_sparse_matrix.data_ptr = (DATATYPE*)_mm_malloc(sizeof(DATATYPE) * (data_num),ANONYMOUSLIB_X86_CACHELINE);
    for( int i = 0 ; i < row_num + 1; i++ ) {
        csr_sparse_matrix.row_ptr[i] = row_array[i];
    }
    for( int i = 0 ; i < data_num ; i++ ) {
        csr_sparse_matrix.column_ptr[i] = column_array[i];

        csr_sparse_matrix.data_ptr[i] = A_array[i];
    }
    #undef ROW_NUM 
    #undef COLUMN_NUM

    return csr_sparse_matrix;
}
#endif
