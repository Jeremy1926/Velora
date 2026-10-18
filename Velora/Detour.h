#pragma once

#include "MinHook.h"
#include "Scanner.h"

extern "C"
{
#include "buffer.h"
#include "trampoline.h"
}

inline void* Detour(const TCHAR* Name, void* Target, void* Function)
{
    static auto Status = MH_Initialize();
    void* Original = nullptr;

    if (!Target)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        return nullptr;
    }

    if (auto Result = MH_CreateHook(Target, Function, &Original); Result != MH_OK)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not hook %s at 0x%llX (%hs)"), Name, (uint8_t*)Target - Scanner::Base(), MH_StatusToString(Result));
    }
    else
    {
        UE_LOG(LogVelora, Log, TEXT("Hooked %s at 0x%llX"), Name, (uint8_t*)Target - Scanner::Base());
    }

    return Original;
}

inline void* DetourCall(const TCHAR* Name, const uint8_t* Call, void* Function)
{
    if (!Call)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        return nullptr;
    }

    auto Jump = (JMP_ABS*)AllocateBuffer((LPVOID)Call);
    *Jump = { 0xFF, 0x25, 0, (UINT64)Function };

    DWORD Protect;
    VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), PAGE_EXECUTE_READWRITE, &Protect);

    auto Original = (void*)(Call + 5 + *(int32_t*)(Call + 1));
    *(int32_t*)(Call + 1) = (int32_t)((uint8_t*)Jump - (Call + 5));

    VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), Protect, &Protect);
    FlushInstructionCache(GetCurrentProcess(), Call, 5);

    UE_LOG(LogVelora, Log, TEXT("Hooked %s at call 0x%llX, was 0x%llX"), Name, Call - Scanner::Base(), (uint8_t*)Original - Scanner::Base());

    return Original;
}

inline void* DetourVTable(const TCHAR* Name, void** Slot, void* Function)
{
    DWORD Protect;
    VirtualProtect(Slot, sizeof(void*), PAGE_READWRITE, &Protect);

    auto Original = *Slot;
    *Slot = Function;

    VirtualProtect(Slot, sizeof(void*), Protect, &Protect);

    if (Name)
    {
        UE_LOG(LogVelora, Log, TEXT("Hooked %s at vtable entry 0x%llX, was 0x%llX"), Name, (uint8_t*)Slot - Scanner::Base(), (uint8_t*)Original - Scanner::Base());
    }

    return Original;
}

inline const uint8_t* PatchBranch(const TCHAR* Name, const uint8_t* Branch, const uint8_t* Target)
{
    if (!Branch || !Target)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        return nullptr;
    }

    auto Displacement = (int32_t*)(Branch + 2);

    DWORD Protect;
    VirtualProtect(Displacement, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &Protect);

    *Displacement = (int32_t)(Target - (Branch + 6));

    VirtualProtect(Displacement, sizeof(int32_t), Protect, &Protect);
    FlushInstructionCache(GetCurrentProcess(), Branch, 6);

    UE_LOG(LogVelora, Log, TEXT("Patched %s at 0x%llX"), Name, Branch - Scanner::Base());

    return Branch;
}

inline const uint8_t* Patch(const TCHAR* Name, const uint8_t* Address, uint8_t Byte)
{
    if (!Address)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        return nullptr;
    }

    DWORD Protect;
    VirtualProtect((LPVOID)Address, sizeof(Byte), PAGE_EXECUTE_READWRITE, &Protect);

    *(uint8_t*)Address = Byte;

    VirtualProtect((LPVOID)Address, sizeof(Byte), Protect, &Protect);
    FlushInstructionCache(GetCurrentProcess(), Address, sizeof(Byte));

    UE_LOG(LogVelora, Log, TEXT("Patched %s at 0x%llX"), Name, Address - Scanner::Base());

    return Address;
}

template<typename FunctionType>
inline FunctionType Detour(const TCHAR* Name, const void* Target, FunctionType Function)
{
    void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    FunctionType Original = {};
    auto Trampoline = Detour(Name, (void*)Target, Address);
    memcpy(&Original, &Trampoline, sizeof(Trampoline));

    return Original;
}

template<typename FunctionType>
inline FunctionType DetourCall(const TCHAR* Name, const uint8_t* Call, FunctionType Function)
{
    void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    FunctionType Original = {};
    auto Target = DetourCall(Name, Call, Address);
    memcpy(&Original, &Target, sizeof(Target));

    return Original;
}

template<typename FunctionType>
inline FunctionType DetourVTable(const TCHAR* Name, void** Slot, FunctionType Function)
{
    void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    FunctionType Original = {};
    auto Target = DetourVTable(Name, Slot, Address);
    memcpy(&Original, &Target, sizeof(Target));

    return Original;
}
