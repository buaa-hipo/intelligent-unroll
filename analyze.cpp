#include "analyze.h"
#include <iostream>
int analyze( int * vec ,ShuffleIndexPtr shuffle_index_ptr ) {
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

