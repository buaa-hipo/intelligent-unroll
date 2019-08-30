#ifndef TOOLS_SET_HPP
#define TOOLS_SET_HPP
#include <stdlib.h>
#include "configure.hpp"
#include <string>
#include <set>
#include <vector>
#include <map>
#include <ulimit.h>
#include<climits>
#include <unordered_map>

typedef enum{DisOrder = 0, Inc = 1, Dec = 2, IncContinue = 3, DecContinue = 4 ,Equel = 5} OrderType;
class Info{
    public:
    int mask_;

    OrderType order_type_; 
    virtual int get_mask()const {
        return mask_;
    }
};

typedef union DisorderAddr {
    int64_t data;
    int int_data_vec_[ (VECTOR >> 2)  ];
    int8_t data_vec[VECTOR];
} GatherAddr;

class GatherInfo : public Info{
    public:
    DisorderAddr disorder_addr_;
    int data_index_[VECTOR];
};
class ScatterInfo : public GatherInfo{
};



class ReductionInfo : public Info {
    public:
    DisorderAddr reduction_addr_[ LOG_VECTOR ];
};

void generate_information( 
        const std::set<std::string> & scatter_set,
        const std::set<std::string> & reduction_set,
        const std::set<std::string> & gather_set,
        const std::map<std::string, void*> & name2ptr_map,
        const int table_column_num,
        ////output
        std::map<std::string,GatherInfo*> &gather_map,
        std::map<std::string,ScatterInfo*> &scatter_map,
        std::map<std::string,ReductionInfo*> &reduction_map,
        std::unordered_map<size_t,std::vector<int>> &same_feature_map_,
        std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map
        ); 
#endif
