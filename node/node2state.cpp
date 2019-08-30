#include "node2state.hpp"
#include "type.hpp"
#include "statement.hpp"
#include <stack>
class Node2StateMent{
    private:
       std::map<const Node * , Varience *> _node2var_map;
       std::stack<const Node*> _cal_stack;
    public:
    const std::vector<std::string> &input_name_vec_;
    const std::set<std::string> &iterates_set_;
    const std::map<std::string, Type > & name_type_map_;
    const std::map<std::string,int*>  &gather_name_new_ptr_map_;
    const std::map<std::string,int*>  &reduction_name_new_ptr_map_;
    const std::map<std::string,int*>  &scatter_name_new_ptr_map_;
    const std::map<std::string,void*>  &name_new_ptr_map_;


    std::map<std::string,Varience*> &name_var_map_;
    std::map< std::string , Varience *> &name_varP_varVP_map_;
    std::map< std::string , Varience *> &name_varP_varPV_map_;
    std::map<std::string, Varience*>  &gather_name_new_var_map_;
    std::map<std::string, Varience*>  &reduction_name_new_var_map_;
    std::map<std::string, Varience*>  &scatter_name_new_var_map_;
    std::map<std::string, Varience*>  &name_new_var_map_;
    std::vector<StateMent *> &func_init_state_vec_;
    FuncStatement * &func_state_ptr_;
    Node2StateMent(
        const std::vector<std::string> &input_var_vec,
        const std::set<std::string> &iterates_set,
        const std::map<std::string, Type > & name_type_map,
        const std::map<std::string,int*>  &gather_name_new_ptr_map,
        const std::map<std::string,int*>  &reduction_name_new_ptr_map,
        const std::map<std::string,int*>  &scatter_name_new_ptr_map,
        const std::map<std::string,void*>  &name_new_ptr_map,

        ///output
        std::map<std::string,Varience*> &name_var_map,
        std::map< std::string , Varience *> &name_varP_varVP_map,
        std::map< std::string , Varience *> &name_varP_varPV_map,
        std::map<std::string, Varience*>  &gather_name_new_var_map,
        std::map<std::string, Varience*>  &reduction_name_new_var_map,
        std::map<std::string, Varience*>  &scatter_name_new_var_map,
        std::map<std::string, Varience*>  &name_new_var_map,
        std::vector<StateMent *> &func_init_state_vec,
        FuncStatement * & func_state_ptr
        ) :
        input_name_vec_(input_var_vec),
        iterates_set_(iterates_set),
        name_var_map_(name_var_map),
        name_type_map_(name_type_map),
        gather_name_new_ptr_map_(gather_name_new_ptr_map_),
        reduction_name_new_ptr_map_(reduction_name_new_ptr_map),
        scatter_name_new_ptr_map_(scatter_name_new_ptr_map),
        name_new_ptr_map_(name_new_ptr_map),
        name_varP_varVP_map_(name_varP_varVP_map),
        name_varP_varPV_map_(name_varP_varPV_map),
        gather_name_new_var_map_(gather_name_new_var_map),
        reduction_name_new_var_map_(reduction_name_new_var_map),
        scatter_name_new_var_map_(scatter_name_new_var_map),
        name_new_var_map_(name_new_var_map),
        func_init_state_vec_(func_init_state_vec),
        func_state_ptr_(func_state_ptr)
    {
    }

    
    void generate_func() {
        std::vector<Type> arg_types_vec; 
        for( const auto & var_name : input_name_vec_ ) {
            auto name_type_map_it = name_type_map_.find( var_name );
            CHECK(name_type_map_it != name_type_map_.end()) << "can not find the type of "<< var_name;
            Type type = name_type_map_it->second;
            
            arg_types_vec.push_back( type ); 
        } 
        func_state_ptr_ = new FuncStatement(__void, arg_types_vec);
        const int args_num = arg_types_vec.size();
        for( int i = 0 ; i < args_num; i++ ) {

            Varience * arg_var_tmp = (*func_state_ptr_->get_args())[i];
            name_var_map_[ input_name_vec_[i] ] = arg_var_tmp;

            Varience * name_varP_varVP_var= new Varience( type_scalar_ptr2vector_ptr(   arg_var_tmp->get_type()) );

                
            Varience * name_varP_varPV_var = new Varience( type_scalar_ptr2ptr_vector(   arg_var_tmp->get_type()) );
            func_init_state_vec_.push_back( LetStat::make( name_varP_varVP_var ,BitCast::make( arg_var_tmp, name_varP_varVP_var->get_type() ) ));

            func_init_state_vec_.push_back( LetStat::make( name_varP_varPV_var ,BroadCast::make( arg_var_tmp ) ));

            name_varP_varVP_map_[ input_name_vec_[i] ] = name_varP_varVP_var;

            name_varP_varPV_map_[ input_name_vec_[i] ] = name_varP_varPV_var; 
        }
        for( const auto& it : gather_name_new_ptr_map_ ) {
            Type type_tmp = __int_ptr;
            Varience * var_tmp = new Varience( type_tmp );
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            gather_name_new_var_map_[ it.first ] = var_tmp;
        } 
        for( const auto& it : reduction_name_new_ptr_map_ ) {
            Type type_tmp = __int_ptr;
            Varience * var_tmp = new Varience( type_tmp );
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            reduction_name_new_var_map_[ it.first ] = var_tmp;
        } 
        for( const auto& it : scatter_name_new_ptr_map_ ) {
            Varience * var_tmp;
            if( reduction_name_new_ptr_map_.find( it.first ) != reduction_name_new_ptr_map_.end() ) {
                ////there is a gather
                auto gather_name_new_var_map_it = gather_name_new_var_map_.find( it.first );
                CHECK(gather_name_new_var_map_it != gather_name_new_var_map_.end()) << "can not find gather";
                var_tmp = gather_name_new_var_map_it->second;
            } else {
                Type type_tmp = __int_ptr;
                var_tmp = new Varience( type_tmp );
                Const * ptr_const = new Const( (uint64_t) it.second );
                func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, type_tmp ) ));
            }
            scatter_name_new_var_map_[ it.first ] = var_tmp;
        } 
        
        for( const auto & it :  name_new_ptr_map_) {
            const auto & var_type = name_type_map_.find( it.first );
            CHECK(var_type != name_type_map_.end()) << "can find type of " << it.first;   
            Varience * var_tmp = new Varience( var_type->second );
            Const * ptr_const = new Const( (uint64_t) it.second );
            func_init_state_vec_.push_back( LetStat::make( var_tmp ,BitCast::make( ptr_const, var_type->second ) ));
            name_new_var_map_[ it.first ] = var_tmp;
        } 
    }

    Varience * find_var_from_node_tree(const Node * node_ptr ) {

        auto node_it = _node2var_map.find(node_ptr); 
        Varience * node_var;
        if(node_it == _node2var_map.end()) {

            node_var = new Varience(  node_ptr->get_type() );
            _node2var_map[ node_ptr ] = node_var;
            _cal_stack.push( node_ptr  );
        } else {
            node_var = node_it->second; 
        }
        return node_var;
    }

    StateMent * generate_code_seed(const Node * root_node_ptr ) {
       
       _cal_stack.push(root_node_ptr);
       std::vector<StateMent*> seed_state_vec;
       while( !_cal_stack.empty() ) {
            const Node * top_node_ptr = _cal_stack.top();
            const GatherNode * gather_node = dynamic_cast<const GatherNode*>( top_node_ptr );
            if( gather_node != NULL )  {
                Node * addr_ptr = gather_node->addr_ptr_;
                Node * index_ptr = gather_node->index_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                seed_state_vec.push_back( Gather::make( addr_var,index_var) );
                seed_state_vec.back()->set_index_name( gather_node->index_name_ );
                seed_state_vec.back()->set_addr_name( gather_node->addr_name_ );
            } else {
            const ScatterNode * scatter_node = dynamic_cast<const ScatterNode*>( top_node_ptr );
            if(scatter_node != NULL) {
                Node * addr_ptr = scatter_node->addr_ptr_;
                Node * index_ptr = scatter_node->index_ptr_;
                Node * data_ptr = scatter_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * index_var = find_var_from_node_tree( index_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Scatter::make( addr_var,index_var,data_var ) ); 

                seed_state_vec.back()->set_index_name( scatter_node->index_name_ );
                seed_state_vec.back()->set_addr_name( scatter_node->addr_name_ );
            } else {
            const VarNode * var_node = dynamic_cast<const VarNode*>( top_node_ptr );

            if(var_node != NULL) {
                const std::string & node_name = var_node->node_name_;
                auto it = name_var_map_.find( node_name ) ;
                if( it == name_var_map_.end() ) {
                    auto ind_it = iterates_set_.find( node_name );
                    if(ind_it == iterates_set_.end()) {
                        LOG(FATAL) << node_name << " Undefined";
                    }
                } else {
                    auto var_node_it = _node2var_map.find( var_node ); 
                    if( var_node_it == _node2var_map.end() ) {
                        LOG(FATAL) << "Can not find Varience";
                    }
                    seed_state_vec.push_back( LetStat::make( var_node_it->second , it->second ) ); 
                }
            } else { 
            const LoadNode * load_node = dynamic_cast<const LoadNode*>( top_node_ptr);
            if(load_node != NULL) {
                auto load_node_it = _node2var_map.find( load_node ); 
                if( load_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * addr_ptr = scatter_node->addr_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                seed_state_vec.push_back( LetStat::make( load_node_it->second , Load::make( addr_var ) ) ); 

                seed_state_vec.back()->set_index_name( load_node->index_name_ );
                seed_state_vec.back()->set_addr_name( load_node->addr_name_ );
            } else {
            const AddNode * add_node = dynamic_cast<const AddNode*>( top_node_ptr );
            if( add_node != NULL ) { 
                auto add_node_it = _node2var_map.find( add_node ); 
                if( add_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = add_node->left_node_;
                Node * right_node = add_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( add_node_it->second,Add::make( left_var,right_var ) ) ); 

                seed_state_vec.back()->set_node_name( add_node->node_name_ );

                seed_state_vec.back()->set_index_name( add_node->index_name_ );

            } else {
            const DivNode * div_node = dynamic_cast<const DivNode*>( top_node_ptr );
            if(div_node != NULL) {
                 auto div_node_it = _node2var_map.find( div_node ); 
                if( div_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = div_node->left_node_;
                Node * right_node = div_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( div_node_it->second,Div::make( left_var,right_var ) ) ); 
                 
                seed_state_vec.back()->set_node_name( div_node->node_name_ );

          
            } else {
            
            const MultNode * mult_node = dynamic_cast<const MultNode*>( top_node_ptr );
            if(mult_node != NULL) {
                auto mult_node_it = _node2var_map.find( mult_node ); 
                if( mult_node_it == _node2var_map.end() ) {
                    LOG(FATAL) << "Can not find Varience";
                }
                Node * left_node = mult_node->left_node_;
                Node * right_node = mult_node->right_node_;

                Varience * left_var = find_var_from_node_tree( left_node );

                Varience * right_var = find_var_from_node_tree( right_node );
                seed_state_vec.push_back( LetStat::make( mult_node_it->second,Mul::make( left_var,right_var ) ) );           

                seed_state_vec.back()->set_node_name( mult_node->node_name_ );


            } else {
            const StoreNode * store_node = dynamic_cast<const StoreNode*>( top_node_ptr ); 
            if(store_node != NULL){
                Node * addr_ptr = store_node->addr_ptr_;
                Node * data_ptr = store_node->data_ptr_;
                Varience * addr_var = find_var_from_node_tree( addr_ptr );
                Varience * data_var = find_var_from_node_tree( data_ptr );
                seed_state_vec.push_back( Store::make( addr_var ,data_var ) );

                seed_state_vec.back()->set_node_name( store_node->node_name_ );

                seed_state_vec.back()->set_index_name( store_node->index_name_ );
                seed_state_vec.back()->set_addr_name( store_node->addr_name_ );

            } else {
            LOG(FATAL) << "Unsupported";
            }}}}}}}}
       }
        
        std::vector<StateMent*> * seed_state_vec_reverse = new std::vector<StateMent*>();
        seed_state_vec_reverse->resize( seed_state_vec.size(), nullptr );
        std::vector<StateMent*>::reverse_iterator rit = seed_state_vec.rbegin();
        for ( int i = 0 ; rit!= seed_state_vec.rend(); ++rit,i++)
            (*seed_state_vec_reverse)[i] = *rit;
        return Block::make( seed_state_vec_reverse ); 
    }
};

void node_tree2state( 
        const std::vector<std::string> &input_var_vec,
        const std::set<std::string> &iterates_set,
        const std::map<std::string, Type > & name_type_map,
        const std::map<std::string,int*>  &gather_name_new_ptr_map,
        const std::map<std::string,int*>  &reduction_name_new_ptr_map,
        const std::map<std::string,int*>  &scatter_name_new_ptr_map,
        const std::map<std::string,void*>  &name_new_ptr_map,

        ///output
        std::map<std::string,Varience*> &name_var_map,
        std::map< std::string , Varience *> &name_varP_varVP_map,
        std::map< std::string , Varience *> &name_varP_varPV_map,
        std::map<std::string, Varience*>  &gather_name_new_var_map,
        std::map<std::string, Varience*>  &reduction_name_new_var_map,
        std::map<std::string, Varience*>  &scatter_name_new_var_map,
        std::map<std::string, Varience*>  &name_new_var_map,
        std::vector<StateMent *> &func_init_state_vec,
        StateMent * & calculate_state,
        FuncStatement * &func_state_ptr,
        const Node * node_ptr
        ) {
         
        Node2StateMent node2state = Node2StateMent(
                input_var_vec,
                iterates_set,
                name_type_map,
                gather_name_new_ptr_map,
                reduction_name_new_ptr_map,
                scatter_name_new_ptr_map,
                name_new_ptr_map,
                name_var_map,
                name_varP_varVP_map,
                name_varP_varPV_map,
                gather_name_new_var_map,
                reduction_name_new_var_map,
                scatter_name_new_var_map,
                name_new_var_map,
                func_init_state_vec,
                func_state_ptr
                );
        node2state.generate_func();
        calculate_state = node2state.generate_code_seed(node_ptr);
}
