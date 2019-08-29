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
        delete * state_vec;
        return Block::make( new_state_vec ); 
    } else {
        new_state_vec->clear();
        delete *new_state_vec;
        return stat;
    }


}
StateMent * FormulationPass::pass_(LetStat * stat) {
    Varience * res_var = stat->get_res();
    StateMent * expr_state = stat->get_expt();
    Varience * expr_var = dynamic_cast<Varience*>(expr_state);
    if(expr_var != NULL) {
        arg2var_map_[ res_var ] = expr_var;
        delete stat;
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
    if(addr_var != NULL) {
        auto arg_var_map_it = arg_var_map_.find( addr_var );
        CHECK(arg_var_map_it != arg_var_map_.end()) << "Can not find " << addr_state;
        auto varP_varPV_map_it = varP_varPV_map_.find( arg_var_map_it->second );
        addr_state_new = varP_varPV_map_it->second;
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

    if(addr_var != NULL) {
        auto arg_var_map_it = arg_var_map_.find( addr_var );
        CHECK(arg_var_map_it != arg_var_map_.end()) << "Can not find " << addr_state;
        auto varP_varVP_map_it = varP_varVP_map_.find( arg_var_map_it->second );
        addr_state_new = varP_varVP_map_it->second;
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

    if(addr_var != NULL) {
        auto arg_var_map_it = arg_var_map_.find( addr_var );
        CHECK(arg_var_map_it != arg_var_map_.end()) << "Can not find " << addr_state;
        auto varP_varVP_map_it = varP_varVP_map_.find( arg_var_map_it->second );
        addr_state_new = varP_varVP_map_it->second;
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

    if(addr_var != NULL) {
        auto arg_var_map_it = arg_var_map_.find( addr_var );
        CHECK(arg_var_map_it != arg_var_map_.end()) << "Can not find " << addr_state;
        auto varP_varVP_map_it = varP_varVP_map_.find( arg_var_map_it->second );
        addr_state_new = varP_varVP_map_it->second;
        StateMent * data_state_new = pass(data_state);

        StateMent * index_state_new = pass(index_state);
        return Scatter::make(addr_state_new, index_state_new,data_state_new);
    } else {
        return StateMentPass::pass_(stat); 
    }    
}
