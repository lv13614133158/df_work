#ifndef __PLATFOR_TYPE_H__
#define __PLATFOR_TYPE_H__
#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Define System Bits
*/ 
#define IS_OS_64 0
#define SK_NUM32_MAX  4294967295UL

/**
 * Define variables
*/
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#ifndef bool
#define bool unsigned char
#endif

#ifndef uint8
typedef unsigned char  uint8;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif
#ifndef uint32
typedef unsigned int   uint32;
#endif
#ifndef uint64
typedef unsigned long long uint64;
#endif

#ifndef uint8
typedef signed char  sint8;
#endif
#ifndef uint16
typedef signed short sint16;
#endif
#ifndef uint32
typedef signed int   sint32;
#endif
#ifndef uint64
typedef signed long long sint64;
#endif
#if IS_OS_64
#ifndef size_t
#define size_t unsigned long 
#endif
#ifndef ssize_t
#define ssize_t signed long
#endif
#else
#ifndef size_t
#define size_t unsigned int
#endif
#ifndef ssize_t
#define ssize_t signed int
#endif
#endif

/**
 * Define simple calculations
*/
#define SK_MAX(a,b)          ((a)>(b) ? (a):(b))
#define SK_MIN(a,b)          ((a)>(b) ? (b):(a))
#define SK_CHECKBIT(a, b)       (a>>b & 0x01)
#define GET_VALUE8(value, n) ((value>>n) & 0xFF)
#define SET_VALUE8(value, n) (value & (0xFF<<n))


/**
 * Test OS bit
*/
typedef struct dummy{
    void *p;
    unsigned char slot;
}dummy_t;
#define OFFSET(type, member)  ((size_t)&(((type *)0)->member))
#define IS_OS_64BIT (OFFSET(dummy_t,slot) == 8) 
#define IS_OS_32BIT (OFFSET(dummy_t,slot) == 4) 

#ifdef __cplusplus
}
#endif

#endif