#pragma once

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef signed int int32;
typedef wchar_t WIDECHAR;
typedef WIDECHAR TCHAR;

#define FORCEINLINE __forceinline
#define FORCENOINLINE __declspec(noinline)

#if !defined(TEXT)
    #define TEXT_PASTE(x) L ## x
    #define TEXT(x) TEXT_PASTE(x)
#endif
