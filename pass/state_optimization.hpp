#ifndef STATE_OPTIMIZATION_HPP
#define STATE_OPTIMIZATION_HPP

class OptimizationPass : StateMentPass{
    private:
    public:
    const std::map<std::string,GatherInfo*> &gather_map_;

    const std::map<std::string,Varience*> &gather_info_var_map_;

    const std::map<std::string,ScatterInfo*> &scatter_map_;
    const std::map<std::string,ReductionInfo*> &reduction_map_;
    const int index_;
    const int circle_num_;
    OptimizationPass(
        const std::map<std::string,GatherInfo*> & gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const int index,
        const int circle_num
            ):
        gather_map_( gather_map ),scatter_map_(scatter_map),reduction_map_(reduction_map),index_(index),circle_num_(circle_num) {

    }

virtual    StateMent* pass_(Block * stat ) ;
virtual    StateMent* pass_(Gather * stat   ) ;
virtual    StateMent* pass_(Add * stat   );
virtual    StateMent* pass_(Scatter * stat  ) ;
}
class OptimizationInnerReducePass : OptimizationPass {
    public:
    const int inner_circle_num_;
    OptimizationInnerReducePass(
        const std::map<std::string,GatherInfo*> & gather_map,
        const std::map<std::string,ScatterInfo*> &scatter_map,
        const std::map<std::string,ReductionInfo*> &reduction_map,
        const int index,
        const int circle_num,
        const int inner_circle_num,
        ): OptimizationPass( gather_map,scatter_map,reduction_map,index,circle_num ),inner_circle_num_(inner_circle_num) {
    }
    
    virtual    StateMent* pass_(Block * stat ) ;
}


void optimization_code_tree( 
        std::map<std::string,GatherInfo*> gather_map,
        std::map<std::string,ScatterInfo*> scatter_map,
        std::map<std::string,ReductionInfo*> reduction_map,
        std::unordered_map<size_t,std::vector<int>> same_feature_map,
    std::unordered_map<size_t,std::vector<std::pair<int,int>>>same_feature_range_map,
        StateMent * code_seed
        ) {
    std::vector<StateMent*> state_vec; 
       for( auto it : same_feature_map ) {
            const std::vector<int> & index_vec = it->second;
            const int index_vec_num = index_vec.size();
            OptimizationPass * optimization_pass_ptr = new OptimizationPass( gather_map, scatter_map, reduction_map, index_vec[0],index_vec_num);
            state_vec.push_back(optimization_pass_ptr->pass( code_seed  ));
       }
       for( auto it : same_feature_range_map ) {
            const std::vector<int> & index_vec = it->second;
            const int index_vec_num = index_vec.size();
            OptimizationInnerReducePass * optimization_pass_ptr = new OptimizationInnerReducePass( gather_map, scatter_map, reduction_map, index_vec[0]->first,index_vec_num);
            state_vec.push_back(optimization_pass_ptr->pass( code_seed  ));
       }

}
#endif
