#include "pch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
    static auto Realloc = Anchor::Function<void* (*)(void*, SIZE_T, uint32)>(TEXT("FMemory::Realloc"))
        .Pattern("4C 8B D1 48 8B 0D ? ? ? ? 48 85 C9 75 08 49 8B CA E9 ? ? ? ? 48 8B 01 45 8B C8 4C 8B C2 49 8B D2 48 FF 60 18")
        .Or()
        .Pattern("48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B CB 4C 8B C7 48 8B D6 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF 60 18")
        .Start()
        .Or()
        .Pattern("48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B CB 4C 8B C7 48 8B D6 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF 60 20")
        .Start()
        .Get();

    return Realloc(Original, Count, Alignment);
}

void FMemory::Free(void* Original)
{
    static auto Free = Anchor::Function<void (*)(void*)>(TEXT("FMemory::Free"))
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Get();

    Free(Original);
}

SIZE_T FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment)
{
    static auto QuantizeSize = Anchor::Function<SIZE_T (*)(SIZE_T, uint32)>(TEXT("FMemory::QuantizeSize"))
        .Pattern("4C 8B 0D ? ? ? ? 4D 85 C9 75 04 48 8B C1 C3 49 8B 01 44 8B C2 48 8B D1 49 8B C9 48 FF 60 28")
        .Or()
        .Pattern("4C 8B 0D ? ? ? ? 4D 85 C9 75 04 48 8B C1 C3 49 8B 01 44 8B C2 48 8B D1 49 8B C9 48 FF 60 38")
        .Optional()
        .Get();

    return QuantizeSize ? QuantizeSize(Count, Alignment) : Count;
}
