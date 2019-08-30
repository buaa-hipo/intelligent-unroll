#include "state_formulation.hpp"

#include "state_pass.hpp"
class FormulationPass : public StateMentPass{
    private:
    StateMent * nop_;
    std::map<Varience*,Varience*> arg2var_map_;
    public:
    const std::map<std::string, Varience *> &name_varP_varVP_map_;

    const std::map<std::string, Varience *> &name_varP_varPV_map_;

    FormulationPass(
        const std::map<std::string, Varience *> &name_varP_varVP_map,
        const std::map<std::string, Varience *> &name_varP_varPV_map):
       name_varP_varVP_map_(name_varP_varVP_map),
       name_varP_varPV_map_(name_varP_varPV_map)
    {
        nop_ = Nop::make();
    }
virtual    StateMent* pass_(Block * stat ) ;
virtual    StateMent* pass_(LetStat * stat ) ;
virtual    StateMent* pass_(Gather * stat   ) ;
virtual    StateMent* pass_(Load * stat);
virtual    StateMent* pass_(Store * stat  ) ;
virtual    StateMent* pass_(Scatter * stat  ) ;
};
StateMent * FormulationPass::pass_(Block * stat) {
    std::vector<StateMent* > * state_vec = stat->get_stat_vec();
    std::vector<StateMent* > * new_state_vec = new std::vector<StateMent*>() ;
    bool is_change = false;
    for(int i = 0 ; i < state_vec->size() ; i++) {
        StateMent * new_state = pass((*state_vec)[i]);
        if( new_state == nop_ ) {
            is_change = true;
        } else
            (*new_state_vec).push_back(new_state); 
    }
    if(is_change) {
        state_vec->clear();
        return Block::make( new_state_vec ); 
    } else {
        new_state_vec->clear();
        return stat;
    }


}
StateMent * FormulationPass::pass_(LetStat * stat) {
    Varience * res_var = stat->get_res();
    StateMent * expr_state = stat->get_expr();
    Varience * expr_var = dynamic_cast<Varience*>(expr_state);
    if(expr_var != NULL && name_varP_varVP_map_.find(expr_var->node_name_) != name_varP_varVP_map_.end() ) {
        return nop_;
    } else {
        res_var->set_type( expr_state->get_type() );
        return LetStat::make( res_var,expr_state );
    } 
}
StateMent * FormulationPass::pass_(Gather * stat) {
    StateMent * addr_state = stat->get_addr();
    
    StateMent * index_state = stat->get_index();
    
    Varience * addr_var = dynamic_cast<Varience*>(addr_state);
    StateMent * addr_state_new;
    StateMent * index_state_new;

    std::string  addr_name = stat->addr_name_;
    if(addr_var != NULL) {
        auto name_varP_varPV_map_it = name_varP_varPV_map_.find( addr_name );
        addr_state_new = name_varP_varPV_map_it->second;
        index_state_new = pass( index_state );
        return Gather::make(addr_state_new,index_state_new);
    } else {
        return StateMentPass::pass_(stat); 
    }
}
StateMent * FormulationPass::pass_(Load * stat) { 
    StateMent * addr_state = stat->get_addr();
    Varience * addr_var = dynamic_cast<Varience*>(addr_state);
    StateMent * addr_state_new;
    std::string  addr_name = stat->addr_name_;
    
    if(addr_var != NULL) {
        auto name_varP_varVP_map_it = name_varP_varVP_map_.find( addr_name );
        addr_state_new = name_varP_varVP_map_it->second;
        return Load::make(addr_state_new);
    } else {
        return StateMentPass::pass_(stat); 
    }
}
StateMent * FormulationPass::pass_(Store * stat) {
    StateMent * addr_state = stat->get_addr();
    StateMent * data_state = stat->get_data();
    Varience * addr_var = dynamic_cast<Varience*>(addr_state);
    StateMent * addr_state_new;

    std::string  addr_name = stat->addr_name_;
    if(addr_var != NULL) {
        auto name_varP_varVP_map_it = name_varP_varVP_map_.find( addr_name );
        addr_state_new = name_varP_varVP_map_it->second;
        StateMent * data_state_new = pass(data_state);
        return Store::make(addr_state_new, data_state_new);
    } else {
        return StateMentPass::pass_(stat); 
    } 
}
StateMent * FormulationPass::pass_(Scatter * stat) {
    StateMent * addr_state = stat->get_addr();
    StateMent * data_state = stat->get_data();

    StateMent * index_state = stat->get_index();
    Varience * addr_var = dynamic_cast<Varience*>(addr_state);
    StateMent * addr_state_new;

    std::string  addr_name = stat->addr_name_;
    if(addr_var != NULL) {
        auto name_varP_varPV_map_it = name_varP_varPV_map_.find( addr_name );
        addr_state_new = name_varP_varPV_map_it->second;
        StateMent * data_state_new = pass(data_state);

        StateMent * index_state_new = pass(index_state);
        return Scatter::make(addr_state_new, index_state_new,data_state_new);
    } else {
        return StateMentPass::pass_(stat); 
    }    
}
StateMent * formulation_state( 
        const std::map<std::string, Varience *> &name_varP_varVP_map,
        const std::map<std::string, Varience *> &name_varP_varPV_map,
        StateMent * state ) {
    FormulationPass formulation_pass( name_varP_varVP_map,name_varP_varPV_map );
    return formulation_pass.pass(state);
}
