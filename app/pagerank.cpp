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
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        PageRankStructurePtr page_rank_structure_ptr = pagerank_read( argv[1]);
  
    
    float * sum = page_rank_structure_ptr->sum;

    float * rank = page_rank_structure_ptr->rank;

    int * n1 = page_rank_structure_ptr->n1;

    int * n2 = page_rank_structure_ptr->n2;
    float * nneibor = page_rank_structure_ptr->nneibor;
    int nedges = page_rank_structure_ptr->nedges;
    int nnodes = page_rank_structure_ptr->nnodes;
    float * sum_bak = SIMPLE_MALLOC( float, nnodes );

    float * sum_time = SIMPLE_MALLOC( float, nnodes );
    for( int i = 0 ; i < nnodes ; i++ )
        sum_bak[i] = sum[i];

    std::string pagerank_str = 
    "input: int * n1,   \
            int * n2,\
            float * rank,\
            float * nneibor\
     output:float * sum \
     lambda i : \
            sum[ n2[i] ] += rank[n1[i]] \
            / nneibor[n1[i]]\
            ";
    std::map<std::string,void*> name2ptr_map;
    name2ptr_map[ "n1" ] = n1;

    name2ptr_map[ "n2" ] = n2;
    name2ptr_map[ "rank" ] = rank;
    name2ptr_map[ "nneibor" ] = nneibor;
    name2ptr_map[ "sum" ] = sum;
#define VECTOR 16
    LOG(INFO) << nedges/VECTOR;
    uint64_t func_int64 = compiler( pagerank_str,name2ptr_map,nedges/VECTOR );
    using FuncType = int(*)( float*,int*,int*,float*,float*);
    FuncType func = (FuncType)(func_int64);
    Timer::startTimer("aot");
    for(int j=0;j<nedges;j++) {
        int nx = n1[j];
        int ny = n2[j];
        sum_bak[ny] += rank[nx] / nneibor[nx];
    }

    Timer::endTimer("aot");

    Timer::printTimer("aot");
    
    func( sum, n1,n2,rank,nneibor );
    LOG(INFO) << nedges / VECTOR * VECTOR;
    for( int i = (nedges / VECTOR * VECTOR) ; i < nedges ; i++ ) {
        sum[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
    }

#define WORM_TIMES 50
     for( int i = 0 ; i < WORM_TIMES ; i++ ) {
        func( sum_time, n1,n2,rank,nneibor );
        for( int i = (nedges / VECTOR * VECTOR) ; i < nedges ; i++ ) {
            sum_time[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
        }
     }

#define TIMES 1000
    Timer::startTimer("jit");
     for( int i = 0 ; i < TIMES ; i++ ){
        func( sum_time, n1,n2,rank,nneibor );
        for( int i = (nedges / VECTOR * VECTOR) ; i < nedges ; i++ ) {
            sum_time[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
        }
     }

    Timer::endTimer("jit");
    Timer::printTimer("jit",TIMES);
    Timer::printGFLOPS( "jit", nedges * 2 , TIMES );
    if(!check_equal( sum_bak, sum, nnodes )) {
        return 1;
    }
    return 0;
}
