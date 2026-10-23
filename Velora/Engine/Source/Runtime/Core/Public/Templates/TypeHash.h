#pragma once

#include "HAL/Platform.h"
#include <type_traits>

inline uint32 HashCombine(uint32 A, uint32 C)
{
    uint32 B = 0x9e3779b9;
    A += B;

    A -= B; A -= C; A ^= (C >> 13);
    B -= C; B -= A; B ^= (A << 8);
    C -= A; C -= B; C ^= (B >> 13);
    A -= B; A -= C; A ^= (C >> 12);
    B -= C; B -= A; B ^= (A << 16);
    C -= A; C -= B; C ^= (B >> 5);
    A -= B; A -= C; A ^= (C >> 3);
    B -= C; B -= A; B ^= (A << 10);
    C -= A; C -= B; C ^= (B >> 15);

    return C;
}

inline uint32 PointerHash(const void* Key, uint32 C = 0)
{
    auto PtrInt = reinterpret_cast<UPTRINT>(Key) >> 4;

    return HashCombine((uint32)PtrInt, C);
}

inline uint32 GetTypeHash(const int32 A)
{
    return A;
}

template<typename EnumType> requires std::is_enum_v<EnumType>
inline uint32 GetTypeHash(EnumType E)
{
    return GetTypeHash((int32)(std::underlying_type_t<EnumType>)E);
}

inline uint32 GetTypeHash(const void* A)
{
    return PointerHash(A);
}
