#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MinHook.h>
#include <Anchor/Anchor.h>

extern "C"
{
#include <buffer.h>
#include <trampoline.h>
}

namespace Anchor
{
    namespace Private
    {
        static const uint8_t* HookedCalls[256][2];
        static int32_t NumHookedCalls = 0;

        const uint8_t* Unhooked(const uint8_t* Target)
        {
            for (int32_t Index = 0; Index < NumHookedCalls; Index++)
            {
                if (HookedCalls[Index][0] == Target)
                    return HookedCalls[Index][1];
            }

            return Target;
        }
    }

    void* Detour(const wchar_t* Name, void* Target, void* Function)
    {
        static auto Status = MH_Initialize();
        void* Original = nullptr;

        if (!Target)
        {
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);
            return nullptr;
        }

        if (auto Result = MH_CreateHook(Target, Function, &Original); Result != MH_OK)
            Private::Log(ELogLevel::Error, L"Could not hook %s at 0x%llX (%hs)", Name, (uint8_t*)Target - Base(), MH_StatusToString(Result));
        else
            Private::Log(ELogLevel::Log, L"Hooked %s at 0x%llX", Name, (uint8_t*)Target - Base());

        return Original;
    }

    void* DetourCall(const wchar_t* Name, const uint8_t* Call, void* Function)
    {
        if (!Call)
        {
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);
            return nullptr;
        }

        auto Jump = (JMP_ABS*)AllocateBuffer((LPVOID)Call);
        *Jump = { 0xFF, 0x25, 0, (UINT64)Function };

        DWORD Protect;
        VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), PAGE_EXECUTE_READWRITE, &Protect);

        auto Original = (void*)(Call + 5 + *(int32_t*)(Call + 1));
        *(int32_t*)(Call + 1) = (int32_t)((uint8_t*)Jump - (Call + 5));

        if (Private::NumHookedCalls < 256)
        {
            Private::HookedCalls[Private::NumHookedCalls][0] = (const uint8_t*)Jump;
            Private::HookedCalls[Private::NumHookedCalls++][1] = (const uint8_t*)Private::Unhooked((const uint8_t*)Original);
        }

        VirtualProtect((LPVOID)(Call + 1), sizeof(int32_t), Protect, &Protect);
        FlushInstructionCache(GetCurrentProcess(), Call, 5);

        Private::Log(ELogLevel::Log, L"Hooked %s at call 0x%llX, was 0x%llX", Name, Call - Base(), (uint8_t*)Original - Base());

        return Original;
    }

    void* DetourVTable(const wchar_t* Name, void** VTable, int64_t Slot, void* Function)
    {
        if (!IsSlot(Slot))
        {
            if (Name)
                Private::Log(ELogLevel::Error, L"Could not find %s", Name);

            return nullptr;
        }

        auto Entry = VTable + Slot / sizeof(void*);

        DWORD Protect;
        VirtualProtect(Entry, sizeof(void*), PAGE_READWRITE, &Protect);

        auto Original = *Entry;
        *Entry = Function;

        VirtualProtect(Entry, sizeof(void*), Protect, &Protect);

        if (Name)
            Private::Log(ELogLevel::Log, L"Hooked %s at vtable entry 0x%llX, was 0x%llX", Name, (uint8_t*)Entry - Base(), (uint8_t*)Original - Base());

        return Original;
    }

    const uint8_t* PatchBranch(const wchar_t* Name, const uint8_t* Branch, const uint8_t* Target)
    {
        if (!Branch || !Target)
        {
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);
            return nullptr;
        }

        auto Displacement = (int32_t*)(Branch + 2);

        DWORD Protect;
        VirtualProtect(Displacement, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &Protect);

        *Displacement = (int32_t)(Target - (Branch + 6));

        VirtualProtect(Displacement, sizeof(int32_t), Protect, &Protect);
        FlushInstructionCache(GetCurrentProcess(), Branch, 6);

        Private::Log(ELogLevel::Log, L"Patched %s at 0x%llX", Name, Branch - Base());

        return Branch;
    }

    const uint8_t* Patch(const wchar_t* Name, const uint8_t* Address, std::initializer_list<uint8_t> Bytes)
    {
        if (!Address)
        {
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);
            return nullptr;
        }

        DWORD Protect;
        VirtualProtect((LPVOID)Address, Bytes.size(), PAGE_EXECUTE_READWRITE, &Protect);

        memcpy((uint8_t*)Address, Bytes.begin(), Bytes.size());

        VirtualProtect((LPVOID)Address, Bytes.size(), Protect, &Protect);
        FlushInstructionCache(GetCurrentProcess(), Address, Bytes.size());

        Private::Log(ELogLevel::Log, L"Patched %s at 0x%llX", Name, Address - Base());

        return Address;
    }
}
