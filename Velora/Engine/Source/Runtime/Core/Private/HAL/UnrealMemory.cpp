#include "pch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"

#pragma warning(disable: 4074)
#pragma init_seg(compiler)

static auto ResizeGrow = Scanner::String(TEXT("Illegal call to StaticFindObject() while collecting garbage!")).Start().Find("xor edx, edx").Call();

void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
    static auto Realloc = (void* (*)(void*, SIZE_T, uint32))ResizeGrow.Call(2).Get(TEXT("FMemory::Realloc"));

    return Realloc(Original, Count, Alignment);
}

void FMemory::Free(void* Original)
{
    static auto Free = (void (*)(void*))Scanner::String(TEXT("Illegal call to StaticFindObject() while collecting garbage!")).Start().Find("test rcx, rcx; jz *; call 0x*").Call().Get(TEXT("FMemory::Free"));

    Free(Original);
}

SIZE_T FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment)
{
    static auto QuantizeSize = (SIZE_T (*)(SIZE_T, uint32))ResizeGrow.Call().Get(TEXT("FMemory::QuantizeSize"));

    return QuantizeSize(Count, Alignment);
}
