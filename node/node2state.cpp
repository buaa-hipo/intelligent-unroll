#include "type.hpp"
#include "statement.hpp"
#include <stack>
class Node2StateMent{
    private:
       std::map<Node * , Varience *> _node2var_map;
       std::stack<Node*> _cal_stack;
    public:
    ParseClass * parse_class_ptr_;
    StateMent * func_init_state_;
    FuncStatement * func_state_ptr_;
    std::map<std::string,Varience*> arg_var_map_;
    std::map<Varience*, Varience *> varP_varVP_map_;

    std::map<Varience*, Varience *> varP_varPV_map_;
    Type VarType2Type(VarType var_type) {
        switch( var_type  ) {
            case var_float_ptr:
                return __float_ptr;
            case var_double_ptr:
                return __double_ptr;
            case vat_int_ptr:
                return __int_ptr;
            case var_float:
                return __float;
            case var_double:
                return __double;
            case vat_int:
                return __int;
            default:
                LOG(FATAL) << "Transfer fault";
        }

    }
    
    Node2StateMent( ParseClass * parse_class_ptr ):parse_class_ptr_(parse_class_ptr) {
    }
    void generate_func() {
        const std::vector<std::string> & input_var_vec = parse_class_ptr_->get_input_var_vec();
        std::vector<Type> arg_types_vec; 
        for( const auto & var_name : input_var_vec ) {
            VarType var_type = parse_class_ptr->get_var_type(var_name);
            Type type = VarType2Type( var_type );
            arg_types_vec.push_back( type ); 
        } 
        func_state_ptr_ = new FuncStatement(__void, arg_types_vec);
        const int args_num = arg_types_vec.size();
        std::vector<StateMent*> func_init_state_vec;
        for( int i = 0 ; i < args_num; i++ ) {

            Varience * arg_var_tmp = (*func_state_ptr_->get_args())[i];
            arg_var_map_[ input_var_vec[i] ] = arg_var_tmp;

            varP_varVP_map_[ arg_var_tmp ] = new Varience( type_scalar_ptr2vector_ptr(   arg_var_tmp->get_type()) );

            varP_varPV_map_[ arg_var_tmp ] = new Varience( type_scalar_ptr2ptr_vector(   arg_var_tmp->get_type()) );
            func_init_state_vec.push_back( LetStat::make( varP_varVP_map_[arg_var_tmp] ,BitCast::make( arg_var_tmp, varP_varVP_map_[arg_var_tmp].get_type() ) ));

            func_init_state_vec.push_back( LetStat::make( varP_varPV_map_[arg_var_tmp] ,BroadCast::make( arg_var_tmp, varP_varPV_map_[arg_var_tmp].get_type() ) ));
        }
        func_init_state_ = CombinStatVec( func_init_state_vec );


    }

    Varience * find_var_from_node_tree( Node * node_ptr ) {

        auto node_it = _node2var_map.find(node_ptr); 
        Varience * node_var;
        if(node_it == _node2var_map.end()) {

            node_var = new Varience( VarType2Type( node_ptr->get_type() ) );
            node2var_map[ node_ptr ] = node_var;
            _cal_stack.push( index_ptr  );
        } else {
            node_var = addr_it->second; 
        }
        return node_var;
    }

    StateMent * generate_code_seed(const Node * root_node_ptr ) {
       
       _cal_stack.push(root_node_ptr);
       std::vector<StateMent*> seed_state_vec;
       while( !_cal_stack.empty() ) {
            Node * top_node_ptr = _cal_stack.top();
            GatherNode * gather_node = dynamic_cast<GatherNode*>( top_node_ptr );
            if( gather_node != NULL )  {
                Node * addr_ptr = gather_node->addr_ptr_;
                Node * index_ptr = gather_node->index_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                seed_state_vec.push_back( Gather::make( addr_var,index_var) );
                seed_state_vec.back().set_index_name( gather_node->index_name_ );
                seed_state_vec.back().set_addr_name( gather_node->addr_name_ );
            } else {
            ScatterNode * scatter_node = dynamic_cast<ScatterNode*>( top_node_ptr );
            if(scatter_node != NULL) {
                Node * addr_ptr = scatter_node->addr_ptr_;
                Node * index_ptr = scatter_node->index_ptr_;
                Node * data_ptr = scatter_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Scatter::make( addr_var,index_var,data_var ) ); 

                seed_state_vec.back().set_index_name( scatter_node->index_name_ );
                seed_state_vec.back().set_addr_name( scatter_node->addr_name_ );
            } else {
            VarNode * var_node = dynamic_cast<ScatterNode*>( top_node_ptr );

            if(var_node != NULL) {
                const std::string & node_name = var_node->node_name_;
                auto it = arg_var_map_[ node_name ] ;
                if( it = arg_var_map_.end() ) {
                    auto ind_it = parse_class_ptr_->index_set_.find( node_name );
                    if(ind_it == parse_class_ptr_->index_set_.end()) {
                        LOG(FATAL) << node_name << " Undefined";
                    }
                } else {
                    auto var_node_it = node2var_map.find( var_node ); 
                    if( var_node_it == node2var_map.end() ) {
                        LOG(FATAL) << "Can not find Varience";
                    }
                    seed_state_vec.push_back( LetStat::make( var_node_it->second , it->second ) ); 
                }
            } else { 
            LoadNode * load_node = dynamic_cast<LoadNode*>( top_node_ptr);
            if(load_node != NULL) {
                auto load_node_it = node2var_map.find( load_node ); 
                if( load_node_it == node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * addr_ptr = scatter_node->addr_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                seed_state_vec.push_back( LetStat::make( load_node_it->second , Load::make( addr_var ) ) ); 

                seed_state_vec.back().set_index_name( load_node->index_name_ );
                seed_state_vec.back().set_addr_name( load_node->addr_name_ );
            } else {
            AddNode * add_node = dynamic_cast<AddNode*>( top_node_ptr );
            if( add_node != NULL ) { 
                auto add_node_it = node2var_map.find( add_node ); 
                if( add_node_it == node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = add_node->left_node_;
                Node * right_node = add_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( add_node_it->second,Add::make( left_var,right_var ) ) ); 

                seed_state_vec.back().set_node_name( add_node->node_name_ );

            } else {
            DivNode * div_node = dynamic_cast<DivNode*>( top_node_ptr );
            if(div_node != NULL) {
                 auto div_node_it = node2var_map.find( div_node ); 
                if( div_node_it == node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = div_node->left_node_;
                Node * right_node = div_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( div_node_it->second,Div::make( left_var,right_var ) ) ); 
                 
                 seed_state_vec.back().set_node_name( div_node->node_name_ );

          
            } else {
            
            MultNode * mult_node = dynamic_cast<MultNode*>( top_node_ptr );
            if(mult_node != NULL) {
                auto mult_node_it = node2var_map.find( mult_node ); 
                if( mult_node_it == node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = mult_node->left_node_;
                Node * right_node = mult_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( mult_node_it->second,Mul::make( left_var,right_var ) ) );           

                seed_state_vec.back().set_var_name( mult_node->node_name_ );
                seed_state_vec.back().set_node_name( mult_node->node_name_ );


            } else {
            StoreNode * store_node = dynamic_cast<StoreNode*>( top_node_ptr ) 
            if(store_node != NULL){
                Node * addr_ptr = store_node->addr_ptr_;
                Node * data_ptr = store_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Store::make( addr_var ,data_var ) );

                seed_state_vec.back().set_var_name( store_node->node_name_ );

                seed_state_vec.back().set_index_name( store_node->index_name_ );
                seed_state_vec.back().set_addr_name( store_node->addr_name_ );

            } else {
            LOG(FATAL) << "Unsupported";
            }}}}}}}}
       }
        
        std::vector<StateMent*> * seed_state_vec_reverse = new std::vector<StateMent*>();
        std::vector<StateMent*>::reverse_iterator rit = seed_state_vec.rbegin();
        for ( int i = 0 ; rit!= seed_state_vec.rend(); ++rit,i++)
            (*seed_state_vec_reverse)[i] = rit;
        return Block::make( seed_state_vec_reverse ); 
    }
}

StateMent * node_tree2state( 
        const Node * root_node_ptr,
        std::map<std::string, Varience*>  gather_name_new_ptr_map_,
        std::map<std::string, Varience*>  reduction_name_new_ptr_map_,
        std::map<std::string, Varience*>  scatter_name_new_ptr_map_,
        std::map<std::string, Varience*>  name_new_ptr_map_
        ) {
        
}
