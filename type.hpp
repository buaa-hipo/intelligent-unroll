#ifndef TYPE_HPP
#define TYPE_HPP
#include "configure.hpp"
typedef enum {FLOAT, DOUBLE, INT, VOID,BOOL } DataType;
#define NOT_VEC (1)
#include <sstream>
#include <string>
class Type {
    DataType data_type_;
    int lanes_;
    Type * pointer2type_;
    public:
    Type( Type * pointer2type, int lanes ) : pointer2type_(pointer2type), lanes_(lanes) {
    }
    Type(DataType data_type,  int lanes ) : data_type_(data_type),lanes_(lanes){ 
        pointer2type_ = nullptr;
    }
    Type() {
        pointer2type_ = nullptr;
    }
    DataType get_data_type()const {
        CHECK(pointer2type_==nullptr) << "type is a pointer";
        return data_type_;
    }
    void set_lanes(const int lanes) {

        lanes_ = lanes;
    }

    int get_lanes()const {

        return lanes_;
    }
    Type * get_pointer2type()const {
        return pointer2type_;
    }
    std::string str()const {
        
        stringstream ss ;
        std::string pointer = std::string();
        if( pointer2type_ == nullptr ) {
        switch( data_type_) {
            case FLOAT:
                ss << "float";
                break;
            case DOUBLE:
                ss << "double";
                break;
            case INT :
                ss << "int";
                break;
            case VOID:
                ss << "void";
                break;
            case BOOL:
                ss << "bool";
                break;
            default:
                LOG(FATAL) << "Undef Typeid" << data_type_;
        }
        ss << " ";

        if(lanes_ != 1) {
            ss << " v" << lanes_;
        }
        ss << " ";
        } else {
          pointer = pointer2type_->str() + "*";
          ss << pointer;
          if(lanes_ != 1) {
            ss  << " v" << lanes_;
          }

        } 
        return ss.str();
    }
    inline bool operator==( const Type & t1 ) const {
        bool ret = true;
        if( this->get_pointer2type() != nullptr && t1.get_pointer2type() != nullptr ) {
            return ( this->lanes_ == t1.get_lanes())&&(*(this->get_pointer2type())) == (*(t1.get_pointer2type())) ;
        }
        if( this->get_pointer2type() != nullptr || t1.get_pointer2type() != nullptr ) {
            return false;
        } else {
        if( this->lanes_ == t1.lanes_ &&
            this->data_type_ == t1.data_type_) {
            return true;
        } else {
            return false;
        }
        }
    }
    friend std::ostream& operator<< ( std::ostream & stream, const Type& type )  {
        stream << type.str();
        return stream;
    }
    bool is_pointer() const {
        return pointer2type_ != nullptr;
    }
};
Type __void = Type(VOID,NOT_VEC);
Type __int = Type( INT,NOT_VEC );
Type __float = Type( FLOAT, NOT_VEC );
Type __double = Type( DOUBLE,  NOT_VEC );
Type __double_ptr = Type( &__double,NOT_VEC);
Type __int_ptr = Type( &__int,NOT_VEC);
Type __float_ptr = Type( &__float, NOT_VEC);

Type __double_v4 = Type( DOUBLE,VECTOR4 );
Type __bool_v4 = Type( BOOL, VECTOR4 );
Type __int_v4 = Type( INT,VECTOR4 );
Type __double_v4_pointer = Type( &__double_v4, NOT_VEC );
Type __int_v4_pointer = Type(&__int_v4, NOT_VEC);

Type __double_pointer_v4 = Type( &__double, VECTOR4 );

Type __double_v8 = Type( DOUBLE,VECTOR8 );
Type __bool_v8 = Type( BOOL, VECTOR8);
Type __int_v8 = Type( INT,VECTOR8 );

Type __double_v8_pointer = Type( &__double_v8, NOT_VEC );
Type __int_v8_pointer = Type(&__int_v8, NOT_VEC);
Type __double_pointer_v8 = Type( &__double, VECTOR8 );

Type __int_v16 = Type(INT,VECTOR16);

Type __bool_v16 = Type( BOOL, VECTOR16);
Type __float_v16 = Type(FLOAT,VECTOR16);
Type __float_v16_ptr = Type( &__float_v16, NOT_VEC );

Type __float_ptr_v16 = Type( &__float, VECTOR16 );

Type __int_ptr_v16 = Type( &__int, VECTOR16 );

Type __int_v16_ptr = Type( &__int_v16, NOT_VEC );

Type __float_v = __float_v16;
Type __float_v_ptr = __float_v16_ptr;
Type __double_pointer_v = __double_pointer_v8;

Type __int_vector_type = __int_v8;
Type __double_vector_type = __double_v8;
Type __double_v = __double_v8;
Type __bool_v = __bool_v16;
Type __int_v = __int_v16;
Type __double_v_pointer = __double_v8_pointer;
Type __int_v_ptr = __int_v16_ptr;
Type __float_ptr_v = __float_ptr_v16;

Type __int_ptr_v = __int_ptr_v16;


#endif

