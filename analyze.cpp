#include "analyze.h"
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <mm_malloc.h>
int analyze(const int * vec ,ShuffleIndexPtr shuffle_index_ptr ) {
    int max_len = 1;
    for( int i = 0 ; i < VECTOR; i++ ) {
        int value = vec[i];
        int tmp_max_len = 1;
        for( int j = i + 1 ; j < VECTOR ; j++ ) {
            if(vec[j] == value)
                tmp_max_len++;
        }
        max_len = tmp_max_len > max_len? tmp_max_len:max_len;
    }
    int num ;
    if( 0x8 & (max_len - 1) ) {
        num = 4;
    } else if(0x4 & (max_len- 1)) {
        num = 3;
    } else if(0x2 & (max_len - 1)) {
        num = 2;
    } else if(0x1 & (max_len - 1)){
        num = 1;
    } else {
        num = 0;
    }
    int has_reduce = 0x0;
    for( int i = 0 ; i < num ; i++ ) {
        char * shuffle_index_ptr_tmp = (char*)&shuffle_index_ptr[i];    
        
        for( int i = 0 ; i < VECTOR ; i++ ) {
            int value = vec[i];
            bool has_find = false;

            if( (has_reduce & (0x1<<i)) == 0 ) {
            for( int j = i + 1 ; j < VECTOR ; j++ ) {
                if(value == vec[j]&&(( has_reduce &(0x1<<j))==0) ) {
                    has_find = true;
                    has_reduce |= (0x1<<j);
                    shuffle_index_ptr_tmp[i] = j;
                    break;
                }
            }
            }
            if(!has_find) {
               shuffle_index_ptr_tmp[i] = VECTOR; 
            }
        }
    }
    return num;
}
int** Analyze( ShuffleIndexPtr shuffle_index_ptr , int * mask_vec,int nedges_pack_num, const int * n2,int * addr_num ) {
    int ** addr = (int**)malloc(sizeof(int*) * MASK_NUM );
    for( int i = 0 ; i < MASK_NUM ; i++ ) 
        addr_num[i] = 0;
    for(int j=0 , j_pack = 0 ;j_pack < nedges_pack_num ;j_pack++, j+=VECTOR) {
        const int *ny = &n2[j];
        int num = analyze( ny , &shuffle_index_ptr[j_pack*4]);
        mask_vec[j_pack] = num;
        addr_num[num]++;
    }
    for( int i = 0 ; i < MASK_NUM ; i++ ) { 
        addr[i] = (int*)_mm_malloc(sizeof(int)*addr_num[i],64);
        addr_num[i] = 0;
    }
    for( int j = 0 ; j < nedges_pack_num ; j++ ) {
        int mask = mask_vec[j];
        int index = addr_num[mask];
        addr[mask][index] = j;
        addr_num[mask]++;
    }
    for( int i = 0 ; i < MASK_NUM ; i++ ) {
        printf("%d:%d\n",i,addr_num[i]);
       // for(int j = 0 ; j < addr_num[i]; j++) 
     //       printf("%d ",addr[i][j]);
        printf("\n");
    }
    return addr;
}



