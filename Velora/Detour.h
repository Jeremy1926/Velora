#pragma once

#include "MinHook.h"

inline void* Detour(void* Target, void* Function)
{
    static auto Status = MH_Initialize();
    void* Original = nullptr;

    MH_CreateHook(Target, Function, &Original);

    return Original;
}

template<typename FunctionType>
inline FunctionType Detour(const void* Target, FunctionType Function)
{
    return std::bit_cast<FunctionType>(Detour((void*)Target, std::bit_cast<void*>(Function)));
}
