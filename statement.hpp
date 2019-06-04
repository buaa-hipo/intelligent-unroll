#ifndef STATEMENT_HPP
#define STATEMENT_HPP
#include "type.hpp"
#include "log.h"
#include <vector>

std::map<std::string,int> classname_typeid_map = {
    {"statement",0},
    {"block",1},
    {"for",2},
    {"varience",3},
    {"const",4},
    {"let_stat",5},
    {"inc_addr",6},
    {"nop",7},
    {"scatter",8},
    {"init",9},
    {"gather",10},
    {"load",11},
    {"store",12},
    {"shuffle",13},
    {"reduce",14},
    {"bitcast",15},
    {"binary",16},
    {"Add",17},
    {"Mul",18},
    {"broadcast",19},
    {"print",20},

    {"Minus",21}
    //static constexpr const char* class_name_ = "broadcast";
};
int GetTypeId(std::string class_name) {
    auto typeid_find = classname_typeid_map.find(class_name);
    if( typeid_find == classname_typeid_map.end()) {
        LOG(FATAL)<<"Can not find " << class_name << "\n";
        exit(1);
    } 
    return typeid_find->second;
}
class StateMent {
    protected:
    Type type_;
    public:
    
    static constexpr const char* class_name_ = "statement";
    virtual std::string get_class_name() {
        return class_name_;
    }

    StateMent( )  {
    }
    virtual Type& get_type() {
        LOG(FATAL) << "does not have type";
        return type_;
    } 
};
class Varience :public StateMent{
    std::string name_;
    void * ptr_;
    std::set<std::string> name_set_;
    std::string get_unique_name() {
        static int name_index = 0;
        stringstream ss ;
        ss << "__" << name_index;
        name_index++;
        auto name_set_find_ = name_set_.find( ss.str() );
        while(name_set_find_ != name_set_.end() ) {
            ss.str(std::string());
            ss << "__" << name_index;
            name_index++;
            name_set_find_ = name_set_.find( ss.str() );
        }
        name_set_.insert( ss.str() );
        return ss.str();
    }
    public:
    
    static constexpr const char * class_name_ = "varience";
    Varience() {
        LOG(FATAL) << "please use other constructe functions";
    }
    Varience(const Type &type ) {
        type_ = type;
        name_ = get_unique_name();
    }
    Varience(double * data) {
        type_ = Type( &__double , NOT_VEC);
        ptr_ = reinterpret_cast<void*>(data);
        name_ = get_unique_name();
    }
    Varience(int * data) {
        type_ = Type( &__int,NOT_VEC);
        ptr_ = reinterpret_cast<void*>(data);
        name_ = get_unique_name();
    }
    virtual Type& get_type() {
        return type_;
    } 
    std::string get_type_str() {
        return type_.str();
    }
    std::string get_name()const {
        return name_;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    friend std::ostream& operator<< (std::ostream & stream, const Varience& var) {
        stream << var.get_name();
        return stream;
    }
};
class Nop : public StateMent{

    protected:
        Nop(){
        }
    public:

    static constexpr const char* class_name_ = "nop";
    static StateMent * make() {
            StateMent * stat_ptr = new Nop();
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }

};

class FuncStatement : public StateMent {
    Varience * ret_;
    StateMent * state_;
    std::string func_name_;

    std::vector<Varience * > *args_;
    public:
    static constexpr const char* class_name_ = "func";
    virtual std::string get_class_name() {
        return class_name_;
    }
    FuncStatement( const Type &ret_type, const std::vector<Type> &args_type) {
        ret_ = new Varience(ret_type);
        args_ = new std::vector<Varience*>();
        func_name_ = "function" ;
        const int args_len = args_type.size();
        args_->resize( args_len,nullptr );
        for(int i = 0 ; i < args_len ; i++) {
            (*args_)[i] = new Varience(args_type[i]); 
        }
    }
    const std::vector<Varience*> * get_args() {
        return args_;
    }
    Varience * get_ret() {
        return ret_;
    }
    void set_state(StateMent * state) {
        state_ = state;
        return;
    }
    StateMent * get_state() {
        return state_;
    }
    std::string & get_func_name() {
        return func_name_;
    }
};

class Expr :public StateMent {
    public :
    static constexpr const char* class_name_ = "expr";
    virtual std::string get_class_name() {
        return class_name_;
    }

    Expr( )  {
    }
    virtual Type& get_type() {
        return type_;
    } 
};
class Block : public StateMent{

    StateMent* stat1_;
    StateMent* stat2_;
    protected:
        Block(StateMent * stat1,StateMent * stat2) : stat1_(stat1),stat2_(stat2) {
            
        }
    public:

    static constexpr const char* class_name_ = "block";
    static  StateMent * make( StateMent * stat1,StateMent * stat2 ) {
            StateMent * stat_ptr = new Block(stat1,stat2);
            return stat_ptr;
    }

    StateMent * get_stat1() {
        return stat1_;
    }
    StateMent * get_stat2() {
        return stat2_;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }


};
class Print : public StateMent{
    
    Varience * var_;
    protected:
    Print( Varience * var ) : var_(var) {
    }
    public:
    static constexpr const char* class_name_ = "print";
    static StateMent * make( Varience * var )  {
        StateMent * stat_ptr = new Print(var);
        return stat_ptr;
    }
    Varience * get_var() {
        return var_;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }

};
class For : public StateMent {
    Varience * var_;
    StateMent * begin_;
    StateMent * space_;
    StateMent * end_;
    StateMent * stat_;

    protected:
    For(StateMent * begin, StateMent * space, StateMent*end, StateMent * stat):begin_(begin), space_(space),end_(end),stat_(stat){
        var_ = new Varience( __int );
    }
    For(StateMent * begin, StateMent * space, StateMent*end):begin_(begin), space_(space),end_(end){
        var_ = new Varience( __int );
    }
    public:

    static constexpr const char* class_name_ = "for";
    static StateMent * make( StateMent * begin,StateMent * space,StateMent * end ){
         
        StateMent * stat_ptr = new For( begin,space,end);
        return stat_ptr;
    }

    static StateMent * make( StateMent * begin,StateMent * space,StateMent * end, StateMent * stat ){
         
        StateMent * stat_ptr = new For( begin,space,end,stat );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }

    void SetState( StateMent * stat ) {
        stat_ = stat;
    }
    StateMent * get_begin() {
        return begin_;
    }
    StateMent * get_space() {
        return space_;
    } 
    StateMent * get_end() {
        return end_;
    } 
    StateMent * get_stat() {
        return stat_;
    }
    Varience * get_var() {
        return var_;
    }

};
class Const : public Expr{
    void * data_;
    public:

    static constexpr const char * class_name_ = "const";
    Const(int data) {
        type_ = Type(INT,NOT_VEC);
        int * ptr = (int*)malloc(sizeof(int));

        ptr[0] = data;
        data_ = reinterpret_cast<void*>(ptr);
    }
    Const( double data) {
        type_ = Type(DOUBLE,NOT_VEC);
        double * ptr = (double*)malloc(sizeof(double));
        ptr[0] = data;
        data_ = reinterpret_cast<void*>(ptr);
   
    }
    template<typename T>
    Const(T * data, int lanes) {
        if( typeid(T) == typeid(int) ) {
            type_ = Type( INT,  lanes);
        } else if( typeid(T) == typeid(double) ) {
            type_ = Type(DOUBLE, lanes);
        } else if( typeid(T) == typeid(bool) ) {
            type_ = Type(BOOL,lanes);
        } else {
            LOG(FATAL) << "Unsupport Type";
        }
        T * tmp = ( T * )malloc(sizeof(T)*lanes);
        for( int i = 0 ; i < lanes ; i++ ) {
            tmp[i] = data[i];
        }
        data_ = reinterpret_cast<void*>(tmp);
    }

    std::string get_type_str() {
        return type_.str();
    }
    template<typename class_name>
    void print_data(class_name* tmp_d, int lanes, stringstream &ss ) {
        int i;
        for( i = 0; i < lanes - 1; i++  ) {
            ss << tmp_d[i] << ","; 
        }
        ss << tmp_d[i];
    }
    std::string get_data_str() {
        stringstream ss;
        double * tmp_d;
        int * tmp_i;
        const int lanes = type_.get_lanes();
        switch( type_.get_data_type() ) {
            case DOUBLE:
                print_data( reinterpret_cast<double*>(data_),lanes,ss);
                break;
            case INT:
                print_data( reinterpret_cast<int*>(data_),lanes,ss);
                break;
            case BOOL:
                print_data( reinterpret_cast<bool*>(data_),lanes,ss);
                break;
            default:
                break;
        }
        return ss.str();
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    void * get_data() {
        return data_;
    }
};
/*class Provide:public StateMent{
    Varience * res_;
    StateMent * expr_;
    
    protected:
    Provide(Varience * res, StateMent * expr) : res_(res),expr_(expr) {
    }
    public:

    static constexpr const char* class_name_ = "provide";
    static StateMent * make( Varience * res,StateMent * expr) {
            StateMent * stat_ptr = new Provide(res,expr);
            return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    Varience * get_res() {
        return res_;
    }
    StateMent * get_expr() {
        return expr_;
    }


}*/
class LetStat:public StateMent{
    Varience * res_;
    StateMent * expr_;
    bool is_const_;
    protected:
    LetStat(Varience * res, StateMent * expr, bool is_const) : res_(res),expr_(expr),is_const_(is_const) {
        }
    public:

    static constexpr const char* class_name_ = "let_stat";
    static StateMent * make(Varience * res,StateMent * expr, bool is_const=true) {
        const Type & res_type = res->get_type();
        const Type & expr_type = expr->get_type();
        CHECK( res_type == expr_type ) << class_name_ <<": " <<res_type << " <-> " << expr_type << "does not match\n";
        StateMent * stat_ptr = new LetStat(res,expr,is_const);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    Varience * get_res() {
        return res_;
    }
    StateMent * get_expr() {
        return expr_;
    }
    bool get_is_const() {
        return is_const_;
    }

};
class IncAddr : public Expr {
    StateMent* addr_;
    StateMent* inc_;
    
    protected:
    IncAddr(StateMent * addr, StateMent * inc) : addr_(addr),inc_(inc){
        type_ = addr->get_type();
    }
    public:

    static constexpr const char* class_name_ = "inc_addr";
    static StateMent * make( StateMent * addr , StateMent * inc) {
        const Type & addr_type = addr->get_type();
        const Type & data_type = inc->get_type();
        CHECK( (addr_type.is_pointer()) ) << addr_type << " " << data_type << "does not match\n";

            StateMent * stat_ptr = new IncAddr(addr,inc);
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    }
    StateMent * get_inc() {
        return inc_;
    }
};
class Scatter: public StateMent{
    StateMent * addr_;
    StateMent * index_;
    StateMent * data_;
    StateMent * mask_;
    protected:
        Scatter( StateMent * addr,StateMent * index,StateMent * data ,StateMent * mask): addr_(addr),index_(index),data_(data),mask_(mask) {
        }
    public:

    static constexpr const char* class_name_ = "scatter";
    static    StateMent * make( StateMent * addr, StateMent * index,StateMent * data, StateMent * mask ) {
            StateMent * stat_ptr = new Scatter(addr,index,data,mask);
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    }
    StateMent * get_index() {
        return index_;
    }
    StateMent * get_data() {
        return data_;
    }
    StateMent * get_mask() {
        return mask_;
    }
};
class Init : public Expr{
    StateMent * data_;

    public:
        template<typename T>
        Init(T * data,int lanes) {
            data_ = new Const( data,lanes );
            type_ = data_->get_type();
        }
    public:

    static constexpr const char* class_name_ = "init";
        template<typename T>
        static StateMent* make(T * data,int lanes=1) {
            StateMent* stat_ptr = new Init( data ,lanes);
            return stat_ptr;
        } 
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_data() {
        return data_;
    } 
};
class Gather: public Expr{
    StateMent * addr_;
    StateMent * index_;
    StateMent * mask_;
    protected:
        Gather( StateMent * addr,StateMent * index,StateMent* mask ): addr_(addr),index_(index),mask_(mask) {
            Type * type_ptr_tmp = &addr->get_type();

            
            type_ = *type_ptr_tmp->get_pointer2type();
            type_.set_lanes(type_ptr_tmp->get_lanes());
        }
    public:

    static constexpr const char* class_name_ = "gather";
    static  StateMent * make( StateMent * addr, StateMent * index , StateMent * mask) {
            StateMent * stat_ptr = new Gather(addr,index,mask);
            return stat_ptr;
        }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent* get_addr() {
        return addr_;
    }
    StateMent* get_index() {
        return index_;
    }
    StateMent * get_mask() {
        return mask_;
    }

};

class Load : public Expr {
    StateMent * addr_;

    protected:
    Load( StateMent * addr):addr_(addr) {

            Type * type_ptr_tmp = &addr->get_type();
            CHECK( type_ptr_tmp->is_pointer()) << "address should be pointer type\n";
            type_ = Type( *type_ptr_tmp->get_pointer2type());
    }
    public:

    static constexpr const char* class_name_ = "load";
    static StateMent * make( StateMent * addr) {
        StateMent * stat_ptr = new Load( addr );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() {
        return addr_;
    } 
};
class Store :public StateMent {
    StateMent * addr_;
    StateMent * data_;

    protected:
    Store( StateMent * addr, StateMent * data ) : addr_(addr),data_(data){
    }
    public:

    static constexpr const char* class_name_ = "store";
    static StateMent * make(StateMent * addr,StateMent * data) {
        const Type & addr_type = addr->get_type();
        const Type & data_type = data->get_type();
        CHECK( (*addr_type.get_pointer2type()) == data_type ) << addr_type << " <-> " << data_type << "does not match\n";
        StateMent * stat_ptr = new Store( addr,data );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_addr() const {
        return addr_;
    } 
    StateMent * get_data() const {
        return data_;
    } 

};

class Shuffle : public Expr {
    StateMent * v1_;
    StateMent * v2_;
    StateMent * index_;
    protected:
    Shuffle( StateMent *v1,StateMent *v2, StateMent *index ) : v1_(v1),v2_(v2),index_(index) {
        CHECK( v1->get_type() == v2->get_type()) << "the type of v1 and v2 is not equal\n";
        CHECK( index->get_type() == __int_vector_type ) << "the lanes of index is not equal";
        type_ = v1->get_type();
    }
    public:

    static constexpr const char* class_name_ = "shuffle";
    static StateMent * make( StateMent *v1,StateMent *v2, StateMent *index ){ 
        StateMent * stat_ptr = new Shuffle(v1,v2,index);
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
        
    }
    StateMent * get_index() {
        return index_;
    }


};
class BroadCast : public Expr {
    
    StateMent * v1_;

    protected:

    BroadCast( StateMent * v1 ) : v1_(v1){

        Type &  type_ptr_tmp = v1_->get_type();
        type_ = type_ptr_tmp;
        type_.set_lanes(VECTOR);

    }
    public:
    static constexpr const char* class_name_ = "broadcast";
    static StateMent * make( StateMent * v1 ) {
        StateMent * stat_ptr = new BroadCast( v1 );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }

};

class Reduce : public Expr {
    StateMent * v1_;

    protected:

    Reduce( StateMent * v1 ) : v1_(v1){
        Type * type_ptr_tmp = &v1_->get_type();
        type_ = *type_ptr_tmp;
        type_.set_lanes(1);
    }
    public:
    static constexpr const char* class_name_ = "reduce";
    static StateMent * make( StateMent * v1 ) {
        StateMent * stat_ptr = new Reduce( v1 );
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }

};
class BitCast : public Expr {

    StateMent * v1_;
    protected:
    BitCast(StateMent * v1,const Type & to ) :v1_(v1) {
        type_ = to;
    }
    public:

    static constexpr const char* class_name_ = "bitcast";
    static StateMent * make( StateMent * v1,  const Type & to ) {
        StateMent * stat_ptr = new BitCast(v1,  to); 
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    std::string get_type_str() {
        return type_.str();
    }
    StateMent * get_v1() {
        return v1_;
    }

};

class Binary : public Expr {
    protected:
    StateMent * v1_;
    StateMent * v2_;

    Binary( StateMent * v1,StateMent * v2 ) : v1_(v1),v2_(v2) {
        type_ = v1->get_type();
    }
    public:

    static constexpr const char* class_name_ = "binary";
    static StateMent * make( StateMent * v1,StateMent * v2 ) {
        StateMent * stat_ptr = new Binary( v1,v2 ); 
        return stat_ptr;
    }
    virtual std::string get_class_name() {
        return class_name_;
    }
    StateMent * get_v1() {
        return v1_;
    }
    StateMent * get_v2() {
        return v2_;
    }

};
#define BINARY( CLASS_NAME ) \
class CLASS_NAME : public Binary{ \
    protected:\
    CLASS_NAME(  StateMent * v1,StateMent*v2 ): Binary( v1,v2) { \
    } \
    public:\
    static constexpr const char* class_name_ = #CLASS_NAME;\
    static StateMent *make( StateMent* v1,StateMent*v2 ) {\
        StateMent * stat_ptr = new CLASS_NAME( v1,v2);\
        return stat_ptr;\
    }\
    virtual std::string get_class_name() {\
        return class_name_;\
    }\
}

BINARY(Minus);
BINARY(Mul);
BINARY(Add);
#endif
