#pragma once

#include "MinHook.h"

extern "C"
{
#include "buffer.h"
#include "trampoline.h"
}

inline void* Detour(void* Target, void* Function)
{
    static auto Status = MH_Initialize();
    void* Original = nullptr;

    MH_CreateHook(Target, Function, &Original);

    return Original;
}

inline void* DetourCall(const uint8_t* Call, void* Function)
{
    auto Jump = (JMP_ABS*)AllocateBuffer((LPVOID)Call);
    *Jump = { 0xFF, 0x25, 0, (UINT64)Function };

    DWORD Protect;
    VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), PAGE_EXECUTE_READWRITE, &Protect);

    auto Original = (void*)(Call + 5 + *(int32_t*)(Call + 1));
    *(int32_t*)(Call + 1) = (int32_t)((uint8_t*)Jump - (Call + 5));

    VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), Protect, &Protect);
    FlushInstructionCache(GetCurrentProcess(), Call, 5);

    return Original;
}

inline const uint8_t* Patch(const uint8_t* Address, uint8_t Byte)
{
    DWORD Protect;
    VirtualProtect((LPVOID)Address, sizeof(Byte), PAGE_EXECUTE_READWRITE, &Protect);

    *(uint8_t*)Address = Byte;

    VirtualProtect((LPVOID)Address, sizeof(Byte), Protect, &Protect);
    FlushInstructionCache(GetCurrentProcess(), Address, sizeof(Byte));

    return Address;
}

template<typename FunctionType>
inline FunctionType Detour(const void* Target, FunctionType Function)
{
    void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    FunctionType Original = {};
    auto Trampoline = Detour((void*)Target, Address);
    memcpy(&Original, &Trampoline, sizeof(Trampoline));

    return Original;
}

template<typename FunctionType>
inline FunctionType DetourCall(const uint8_t* Call, FunctionType Function)
{
    void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    FunctionType Original = {};
    auto Target = DetourCall(Call, Address);
    memcpy(&Original, &Target, sizeof(Target));

    return Original;
}
