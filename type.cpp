#include "type.hpp"
Type __void = Type(VOID,NOT_VEC);
Type __int = Type( INT,NOT_VEC );
Type __int8 = Type( INT8,NOT_VEC );
Type __float = Type( FLOAT, NOT_VEC );
Type __double = Type( DOUBLE,  NOT_VEC );
Type __double_ptr = Type( &__double,NOT_VEC);
Type __int_ptr = Type( &__int,NOT_VEC);
Type __int_ptr_ptr = Type( &__int_ptr,NOT_VEC);
Type __float_ptr = Type( &__float, NOT_VEC);
Type __int8_ptr = Type( &__int8,NOT_VEC);

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
Type __int8_v16 = Type( INT8,VECTOR16 );
Type __bool_v16 = Type( BOOL, VECTOR16);
Type __float_v16 = Type(FLOAT,VECTOR16);
Type __float_v16_ptr = Type( &__float_v16, NOT_VEC );

Type __float_ptr_v16 = Type( &__float, VECTOR16 );

Type __int_ptr_v16 = Type( &__int, VECTOR16 );

Type __int8_ptr_v16 = Type( &__int8, VECTOR16 );
Type __int_v16_ptr = Type( &__int_v16, NOT_VEC );
Type __int8_v16_ptr = Type( &__int8_v16, NOT_VEC );

Type __float_v = __float_v16;
Type __float_v_ptr = __float_v16_ptr;
Type __double_pointer_v = __double_pointer_v8;

Type __int_vector_type = __int_v8;
Type __double_vector_type = __double_v8;
Type __double_v = __double_v8;
Type __bool_v = __bool_v16;
Type __int_v = __int_v16;
Type __int8_v = __int8_v16;

Type __double_v_pointer = __double_v8_pointer;
Type __int_v_ptr = __int_v16_ptr;
Type __int8_v_ptr = __int8_v16_ptr;

Type __float_ptr_v = __float_ptr_v16;

Type __int_ptr_v = __int_ptr_v16;
Type __int8_ptr_v = __int8_ptr_v16;


