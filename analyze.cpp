#include "analyze.h"
#include <iostream>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <mm_malloc.h>
#include <vector>
#include "log.h"

void AnalyzeGetMask2( int * &row_begin_ptr , int* &index_begin_ptr,Mask2 * &mask_ptr, int & vec_num, int * row_ptr , int * column_ptr, const int row_num, const int column_num) {
    std::vector<Mask2*> mask_vec;
    std::vector<int> row_begin_vec;
    std::vector<int> index_begin_vec;
    int row_begin = 0;
    int index_begin = 0;
    int last_num = 0 ;
    int mask = 0x1;
    int num;
    bool debug_plug = false;
    for( int i = 0 ; i < row_num ; i++ ) {

        while( i < row_num ) {
//            if( i == 2143 )
//                debug_plug = true;
            int cur_num = row_ptr[ i + 1 ] - row_ptr[ i ];
            if( cur_num + last_num >= VECTOR ) {
                if( last_num != 0 ) {
                    num = 1;
                    Mask2 * mask_tmp = new Mask2( num , mask );
                    mask_vec.push_back( mask_tmp ) ;
                    row_begin_vec.push_back( row_begin );

                    index_begin_vec.push_back(index_begin);
//                    if(debug_plug) {
//                        LOG(INFO)  << *mask_tmp << " " << row_begin << " " << index_begin;
//                        debug_plug = false;
//                        exit(1);
//                    }
                }
                
                int top_num ;
                if( last_num != 0 )
                    top_num = VECTOR - last_num ;
                else 
                    top_num = 0;
                num = (cur_num - top_num) / VECTOR ;
                if( num > 0 ) {
                    int middle_num = num * VECTOR;
                    Mask2 * mask_tmp = new Mask2( num , 0x1);
                    row_begin = i;
                    index_begin = (row_ptr[i] + top_num) / VECTOR; 

                    CHECK( ((row_ptr[i]+top_num ) % VECTOR) == 0 ) << "index begin should be some number multiple vector";
                    row_begin_vec.push_back( row_begin );
                    index_begin_vec.push_back( index_begin );
                    mask_vec.push_back( mask_tmp ) ;
                    mask = 0x1 ;

                    last_num = cur_num - middle_num - top_num;
                } else {
                    mask = 0x1;
                    last_num = cur_num - top_num;
                }
                if( last_num != 0 ) {
                    mask |= ( 0x1 << last_num);
                    row_begin = i;
                    index_begin = (row_ptr[i+1] - last_num) /VECTOR ;
                    CHECK( ((row_ptr[i+1] - last_num) % VECTOR) == 0 ) << "index begin should be some number multiple vector";
                } else {
                    row_begin = i + 1;
                    index_begin = (row_ptr[i+1])/ VECTOR;

                    CHECK( ((row_ptr[i+1] ) % VECTOR) == 0 ) << "index begin should be some number multiple vector";
                }

                break;
            } else {
                last_num += cur_num;
                mask |= ( 0x1 << last_num );
                i++;
            }
        }
    }
//    exit(1);
    const int vec_data_num = mask_vec.size();
    mask_ptr = new Mask2[vec_data_num];
    row_begin_ptr = ( int * ) malloc( sizeof(int) * vec_data_num);
    index_begin_ptr = ( int* )malloc( sizeof(int) * vec_data_num);
    for( int i = 0 ; i < vec_data_num ; i++ ) {
        mask_ptr[i] = *mask_vec[i];
        row_begin_ptr[i] = row_begin_vec[i];
        index_begin_ptr[i] = index_begin_vec[i];
    }
    vec_num = vec_data_num;
}
#define LOAD_TO_GATHER_LEVEL 1
    typedef union GatherAddr {
        int64_t data;
        char data_vec[VECTOR];
    } GatherAddr;
    int analyze_gather_addr(const int * column_block_ptr, int64_t &addr) {
        GatherMask gather_addr;
        int column_block[VECTOR];
        for( int v_i = 0 ; v_i < VECTOR; v_i++ ) {
            column_block[v_i] = column_block_ptr[v_i];
        }
        int num = 0;
        while(true) {
            int min = INT_MAX ;
            for( int v_i = 1 ; v_i < VECTOR ; v_i++) {
                min = column_block[v_i] < min ? column_block[v_i] : min;
            }
            if( min == INT_MAX) {
                break;
            }
            num++;
            for( int v_i = 0 ; v_i < VECTOR ; v_i++ ) {
                if( column_block[v_i] < min + VECTOR ) {
                    column_block[v_i] = INT_MAX ;
                    gather_addr.data_vec[ v_i ] = column_block[v_i] - min;
                }
            }
        }
        if( num <= LOAD_TO_GATHER_LEVEL ) {
            addr = gather_addr.data;
            return num;
        } else {
            return -1;
        }
    }


void Analyze(  std::map<Mask2, std::pair<std::vector<int>,std::vector<int>>> &mask_map, std::map<Mask2 , int> & mask_num_map, int & mask_num ,  int * row_ptr , int * column_ptr, const int row_num, const int column_num) {
    int * row_begin_ptr;
    int * index_begin_ptr;
    Mask2 * mask_ptr;
    int vec_num;
    int mask_num_tmp = 0;
    AnalyzeGetMask2( row_begin_ptr, index_begin_ptr,mask_ptr, vec_num, row_ptr, column_ptr, row_num, column_num );
    generate_gather_mask( index_begin_ptr, vec_num, column_ptr );
    Show( mask_ptr, vec_num );
    for( int i = 0 ; i < vec_num; i++ ) {
        auto it = mask_map.find( mask_ptr[i] );
        if( it == mask_map.end() ) {
            mask_map[ mask_ptr[i] ] = std::make_pair(std::vector<int>(1,row_begin_ptr[i]) , std::vector<int>(1,index_begin_ptr[i]));
            mask_num_map[ mask_ptr[i] ] = 1;
            mask_num_tmp++;
        } else {
            (it->second).first.push_back(row_begin_ptr[i]);

            (it->second).second.push_back(index_begin_ptr[i]);
            mask_num_map[mask_ptr[i]]++;
        }
    }
    mask_num = mask_num_tmp;
}
void Show( Mask2 * mask_ptr, const int mask_num ) {
    std::map<Mask2, int> mask_map;
    int debug_i = 0 ;
    for( int i = 0 ; i < mask_num; i++ ) {
        auto it = mask_map.find( mask_ptr[i] );
        if( it == mask_map.end() ) {
            mask_map[mask_ptr[i]] = 1;
        } else {
            it->second++;
        }
    }
    int num = 0 ;
    for( auto it = mask_map.begin(); it != mask_map.end(); it++ ) {
        num++;
        std::cout << it->first << " " << it->second << "\n"; 
    }
    std::cout << num << std::endl;
}





