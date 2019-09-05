/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include "Timers.hpp"
#include "intelligent_unroll.hpp"
#include "util.h"
#include "csr_matrix.h"
#define PRINTINT(x) do {    \
                    printf( #x" %d\n" , (x));fflush(stdout); \
                        } while(0)
void spmv_local(DATATYPE * y_ptr,const DATATYPE * x_ptr,const DATATYPE * data_ptr, const int * column_ptr, const int * row_ptr, const int row_num ) {
    for (int i = 0 ; i < row_num ; i++ ) {
        DATATYPE sum = 0;
        for (int j = row_ptr[i]; j < row_ptr[i+1] ; j++ ) {
            sum += x_ptr[column_ptr[j]] * data_ptr[j];
        }
        y_ptr[i] += sum;
    }
}

void init_vec(DATATYPE * dence_vec_ptr, const int data_num , const DATATYPE data, const bool change = false ) {
    if( change ) {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = i;
        }
    } else {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = data ;

        }
    }
}
template<typename T>
bool check_equal(const T * v1, const T * v2, const int num ) {
    bool flag = true;
    for( int i = 0 ; i < num ; i++ ) {
        if( (v1[i]-v2[i]) > 1e-3 || (v2[i]-v1[i]) > 1e-3 ) {
            flag = false;
           std::cout<< i<< " "  << v1[i]  << " "<< v2[i]<<"\n";
        }
    }
    if(flag) 
        std::cout<<"Correct"<<std::endl;
    else 
        std::cout<<"False"<<std::endl;
    return flag;
}

template<typename T>
void print_vec( T * data_ptr, const int num ) {
    for( int i = 0 ; i < num ; i++ ) {
        std::cout<< data_ptr[i] << " ";
    }
    std::cout<<std::endl;
}
//#define LITTEL_CASE2
int main( int argc , char const * argv[] ) {
    #ifdef LITTEL_CASE
        csrSparseMatrix sparseMatrix = little_test();
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #elif defined LITTEL_CASE2

        csrSparseMatrix sparseMatrix = little_test2(1024 ,1024);
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #else
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        csrSparseMatrixPtr sparseMatrixPtr = matrix_read_csr( argv[1]);
    #endif
  
    
    DATATYPE * data_ptr = sparseMatrixPtr->data_ptr;
    int * column_ptr = sparseMatrixPtr->column_ptr;
    int * row_ptr = sparseMatrixPtr->row_ptr;

    const int data_num = sparseMatrixPtr->data_num;
    const int row_num = sparseMatrixPtr->row_num;
    const int column_num = sparseMatrixPtr->column_num;
    DATATYPE * x_array = SIMPLE_MALLOC( DATATYPE , column_num );
    DATATYPE * y_array = SIMPLE_MALLOC( DATATYPE, row_num );
    DATATYPE * y_array_bak = SIMPLE_MALLOC( DATATYPE , row_num );
    
    DATATYPE * y_array_time = SIMPLE_MALLOC( DATATYPE, row_num );
//    init_vec(x_array,column_num,1);

//    init_vec( x_array, column_num , 1 ,true);
    init_vec( x_array, column_num , 1 ,true);

    //init_vec(x_array,column_num,1);
    init_vec( y_array, row_num , 0 );
    init_vec( y_array_bak, row_num , 0 );

    init_vec( y_array_time, row_num , 0 );
    int * row_ptr_all = SIMPLE_MALLOC( int, data_num );
    for(int row_i = 0 ; row_i < row_num; row_i++) {
        int begin = row_ptr[row_i];
        int end = row_ptr[row_i+1];
        for( int j = begin ; j < end ; j++ ) {
            row_ptr_all[j] = row_i;
        }
    }
    std::string spmv_str = 
    "input: int * n1,   \
            int * n2,\
            float * rank,\
            float * nneighbor\
     output:double * sum \
     lambda i : \
            y_array[ row_ptr[i] ] += data_ptr[i] \
            * x_array[column_ptr[i]]\
            ";
    std::map<std::string,void*> name2ptr_map;
    name2ptr_map[ "row_ptr" ] = row_ptr_all;

    name2ptr_map[ "column_ptr" ] = column_ptr;
    name2ptr_map[ "x_array" ] = x_array;
    name2ptr_map[ "data_ptr" ] = data_ptr;
    name2ptr_map[ "y_array" ] = y_array;
    LOG(INFO) << data_num/VECTOR;
    uint64_t func_int64 = compiler( spmv_str,name2ptr_map,data_num/VECTOR );
    using FuncType = int(*)( double*,int*,int*,double*,double*);
    FuncType func = (FuncType)(func_int64);
    Timer::startTimer("aot");
        spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );

    Timer::endTimer("aot");

    Timer::printTimer("aot");
    
    func( y_array,row_ptr_all, column_ptr, x_array,data_ptr );
    LOG(INFO) << data_num / VECTOR * VECTOR;
    for( int i = (data_num / VECTOR * VECTOR) ; i < data_num ; i++ ) {
        y_array[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
    }
     for( int i = 0 ; i < 50 ; i++ ) {
        func( y_array_time,row_ptr_all, column_ptr, x_array,data_ptr );
        for( int i = data_num / VECTOR * VECTOR ; i < data_num ; i++ ) {

            y_array_time[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
        }
     }

#define TIMES 1000
    Timer::startTimer("jit");
     for( int i = 0 ; i < TIMES ; i++ ){
        func( y_array_time,row_ptr_all, column_ptr, x_array,data_ptr );
        for( int i = data_num / VECTOR * VECTOR ; i < data_num ; i++ ) {

            y_array_time[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
        }

     }

    Timer::endTimer("jit");
    Timer::printTimer("jit",TIMES);
    Timer::printGFLOPS( "jit", data_num * 2 , TIMES );
    if(!check_equal( y_array_bak, y_array, row_num )) {
        return 1;
    }
    return 0;
}
