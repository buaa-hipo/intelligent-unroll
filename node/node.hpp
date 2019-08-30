#ifndef NODE_HPP
#define NODE_HPP
#include "type.hpp"
class Node{
    public:
    std::string node_name_;
    std::string addr_name_;
    std::string index_name_;
    Type type_;
    Node(){
    }
    Node(const Type & type,const std::string & node_name ):
    type_(type),node_name_(node_name)
    {
    }
    Type get_type()const {
        return type_;
    }
    virtual ~Node() = default;
};
class GatherNode : public Node{
    public:
        Node * addr_ptr_;
    Node * index_ptr_;
    GatherNode(Node * addr_ptr, Node * index_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr)
    {
        type_ = *(addr_ptr->get_type().get_pointer2type());
    }
};
class ScatterNode :public Node{
    public:
    Node * addr_ptr_;
    Node * index_ptr_;
    Node * data_ptr_;
    ScatterNode(Node * addr_ptr, Node * index_ptr, Node * data_ptr):addr_ptr_(addr_ptr), index_ptr_(index_ptr),data_ptr_(data_ptr)
    {
        type_ = *(addr_ptr->get_type().get_pointer2type());
    }
};
class VarNode :public Node{
    public:
    VarNode(const Type& type,const std::string &node_name) : Node( type,node_name ) {
    }
};
class LoadNode:public Node{

    public:
    Node * addr_ptr_;

    LoadNode(Node * addr_ptr):addr_ptr_(addr_ptr)
    {

        type_ = *addr_ptr->get_type().get_pointer2type();
    }

};
class BinaryNode:public  Node{
    public:
    Node * left_node_;
    Node * right_node_;

    BinaryNode(Node * left_node,  Node * right_node):left_node_(left_node),right_node_(right_node)
    {
        type_ = left_node_->get_type();
    }

};
#define BINARY_DEFINE(CLASS_NAME) \
    class CLASS_NAME##Node :public BinaryNode { \
        public:\
    CLASS_NAME##Node(Node*left_node,Node * right_node) : BinaryNode(left_node,right_node){\
    } \
}

BINARY_DEFINE(Mult);
BINARY_DEFINE(Div);
BINARY_DEFINE(Add);

class StoreNode: public Node{
    public:
    Node * addr_ptr_;
    Node * data_ptr_;

    StoreNode(Node * addr_ptr,  Node * data_ptr):addr_ptr_(addr_ptr),data_ptr_(data_ptr)
    {
        type_ =  *addr_ptr_->get_type().get_pointer2type() ;
    }

};

#endif
