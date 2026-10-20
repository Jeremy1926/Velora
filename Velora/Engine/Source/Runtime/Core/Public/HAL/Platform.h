#pragma once

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed char int8;
typedef signed short int int16;
typedef signed int int32;
typedef signed long long int64;
typedef unsigned long long SIZE_T;
typedef unsigned long long UPTRINT;
typedef char ANSICHAR;
typedef wchar_t WIDECHAR;
typedef WIDECHAR TCHAR;

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)

#if !defined(TEXT)
    #define TEXT_PASTE(x) L ## x
    #define TEXT(x) TEXT_PASTE(x)
#endif
