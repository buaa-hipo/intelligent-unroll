#ifndef TRANSFORM_DATA_HPP
#define TRANSFORM_DATA_HPP
#include<set>
#include<map>
#include<unordered_map>
#include<vector>
#include "tools_set.hpp"
void transform_data(
    const std::set<std::string> & transform_data_set,
    const std::map<std::string, VarType > & name_type_map,
    const std::map<std::string,void*> & name_ptr_map,
    const std::set<std::string > & gather_set,
    const std::set<std::string> & scatter_set,
    const std::set<std::string> & reduction_set,
    const std::unordered_map<size_t,std::vector<int>> &same_feature_map,
    const std::unordered_map<size_t,std::vector<std::pair<int,int>>>&same_feature_range_map,
    ///output
    std::map<std::string,int*> & gather_name_new_ptr_map,
    std::map<std::string,int*> & reduction_name_new_ptr_map,
    std::map<std::string,int*> & scatter_name_new_ptr_map,
    std::map<std::string,void*> & name_new_ptr_map
    ); 
#endif
