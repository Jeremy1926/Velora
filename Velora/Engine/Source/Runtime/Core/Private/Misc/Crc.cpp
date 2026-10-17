#include "pch.h"
#include "Misc/Crc.h"
#include "Scanner.h"
#include "hde/hde64.h"

uint32 FCrc::Strihash_DEPRECATED(const TCHAR* Data)
{
    static auto Strihash_DEPRECATED = []
    {
        auto Reference = Scanner::FindStringReference(TEXT("match.option.warmupRequiredPlayerCount"));
        const uint8_t* FindId = nullptr;
        bool bKeyCopied = false;

        for (auto Address = Reference; !FindId;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                if (bKeyCopied)
                    FindId = Address + Length + (int32_t)Instruction.imm.imm32;

                bKeyCopied = true;
            }

            Address += Length;
        }

        for (auto Address = FindId;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                return (uint32 (*)(const TCHAR*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    return Strihash_DEPRECATED(Data);
}
