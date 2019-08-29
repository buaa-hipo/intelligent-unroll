#ifndef NODE_HPP
#define NODE_HPP
class Node{

    std::string node_name_;
    std::string addr_name_;
    std::string index_name_;

    public:
    VarType var_type_;
    VarType get_var_type() {
        return var_type_;
    }
};
class GatherNode : Node{
    public:
        Node * addr_ptr_;
    Node * index_ptr_;
    GatherNode(Node * addr_ptr, Node * index_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr)
    {
        var_type_ = GetBasicType(addr_ptr.get_type());
    }
};
class ScatterNode : Node{
    public:
    Node * addr_ptr_;
    Node * index_ptr_;
    Node * data_ptr_;
    ScatterNode(Node * addr_ptr, Node * index_ptr, Node * data_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr),data_ptr_(data_ptr)
    {
        var_type_ = GetBasicType(addr_ptr.get_type());
    }
};
class VarNode : Node{
    public:
    VarNode(VarType var_type, std::string node_name) : var_type_(var_type),node_name_(node_name){
    }
}
class LoadNode: Node{

    Node * addr_ptr_;

    LoadNode(Node * addr_ptr):addr_ptr_(addr_ptr)
    {

        var_type_ = GetBasicType(addr_ptr.get_type());
    }

};
class BinaryNode: Node{
    public:
    Node * left_node_;
    Node * right_node_;

    BinaryNode(Node * left_node,  Node * right_node):left_node_(left_node),right_node_(right_node)
    {
        var_type_ = left_node_.get_type();
    }

};
#define BINARY_DEFINE(CLASS_NAME) \
    class CLASS_NAME##Node : BinaryNode { \
    CLASS_NAME##Node(Node*left_node,Node * right_node) : BinaryNode(left_node,right_node){\
    } \
}

BINARY_DEFINE(Mult);
BINARY_DEFINE(Div);
BINARY_DEFINE(Add);

class StoreNode: Node{
    Node * addr_ptr_;
    Node * data_ptr_;

    StoreNode(Node * addr_ptr,  Node * data_ptr):addr_ptr_(addr_ptr),data_ptr_(data_ptr)
    {
        var_type_ = GetBasicType( addr_ptr_->get_type() );
    }

};

#endif
