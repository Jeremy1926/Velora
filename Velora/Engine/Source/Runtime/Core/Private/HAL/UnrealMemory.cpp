#include "pch.h"
#include "HAL/UnrealMemory.h"
#include "Scanner.h"
#include "hde/hde64.h"

static auto ResizeGrowCalls = []
{
    auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")));
    auto End = Scanner::FindFunctionEnd(Function);
    const uint8_t* ResizeGrow = nullptr;
    bool bOldNumZero = false;

    for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End && !ResizeGrow;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && bOldNumZero)
            ResizeGrow = Address + Length + (int32_t)Instruction.imm.imm32;
        else if (Instruction.opcode == 0x33 && Instruction.modrm == 0xD2)
            bOldNumZero = true;

        Address += Length;
    }

    auto ResizeGrowEnd = Scanner::FindFunctionEnd(Scanner::FindFunction(ResizeGrow));
    std::array<const uint8_t*, 2> Calls = {};
    auto Count = 0ull;

    for (auto Address = ResizeGrow; Address < ResizeGrowEnd && Count < Calls.size();)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8)
            Calls[Count++] = Address + Length + (int32_t)Instruction.imm.imm32;

        Address += Length;
    }

    return Calls;
}();

void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
    static auto Realloc = (void* (*)(void*, SIZE_T, uint32))ResizeGrowCalls[1];

    return Realloc(Original, Count, Alignment);
}

void FMemory::Free(void* Original)
{
    static auto Free = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("Illegal call to StaticFindObject() while collecting garbage!")));
        auto End = Scanner::FindFunctionEnd(Function);
        const uint8_t* Instructions[2] = {};

        for (auto Address = Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && Instructions[0] && !memcmp(Instructions[0], "\x48\x85\xC9", 3) && !memcmp(Instructions[1], "\x74\x05", 2))
                return (void (*)(void*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Instructions[0] = Instructions[1];
            Instructions[1] = Address;
            Address += Length;
        }

        return (void (*)(void*))nullptr;
    }();

    Free(Original);
}

SIZE_T FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment)
{
    static auto QuantizeSize = (SIZE_T (*)(SIZE_T, uint32))ResizeGrowCalls[0];

    return QuantizeSize(Count, Alignment);
}
