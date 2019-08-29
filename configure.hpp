#ifndef CONFIGURE_HPP
#define CONFIGURE_HPP
    #define VECTOR4 4
    #define VECTOR8 8
    #define VECTOR16 16
    #define LOG_VECTOR8 3
    #define VECTOR VECTOR8
    #define _2_POWER_VECTOR (1<<VECTOR)
    #define _2_POWER_VECTOR_MINUS_1 (1 << (VECTOR-1))
    #define GATHER_TYPE_NUM (VECTOR+1)
    #define LOG_VECTOR LOG_VECTOR8
    #define VEC_MASK 0xff  
    #define MASK char
#endif
