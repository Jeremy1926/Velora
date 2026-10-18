#pragma once

#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
#include "Logging/LogMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

extern FLogCategoryBase LogVelora;

namespace Scanner
{
    inline uint8_t* Base()
    {
        return (uint8_t*)GetModuleHandleW(nullptr);
    }

    inline std::span<IMAGE_SECTION_HEADER> Sections()
    {
        auto Headers = (IMAGE_NT_HEADERS*)(Base() + ((IMAGE_DOS_HEADER*)Base())->e_lfanew);

        return { IMAGE_FIRST_SECTION(Headers), Headers->FileHeader.NumberOfSections };
    }

    inline const uint8_t* FindReference(const void* Target)
    {
        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + 6 <= End; Address++)
            {
                if (Address[0] == 0x8D && (Address[1] & 0xC7) == 0x05 && Address + 6 + *(int32_t*)(Address + 2) == Target)
                    return Address;
            }
        }

        return nullptr;
    }

    inline const uint8_t* FindCallReference(const void* Target)
    {
        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + 5 <= End; Address++)
            {
                if ((Address[0] == 0xE8 || Address[0] == 0xE9) && Address + 5 + *(int32_t*)(Address + 1) == Target)
                {
                    UE_LOG(LogVelora, Log, TEXT("Found a call to 0x%llX at 0x%llX"), (const uint8_t*)Target - Base(), Address - Base());
                    return Address;
                }
            }
        }

        UE_LOG(LogVelora, Error, TEXT("Could not find a call to 0x%llX"), (const uint8_t*)Target - Base());
        return nullptr;
    }

    inline const uint8_t* FindStringReference(const wchar_t* String)
    {
        static SRWLOCK Lock = SRWLOCK_INIT;
        static std::unordered_map<std::wstring, const uint8_t*> References;

        AcquireSRWLockShared(&Lock);
        auto Found = References.find(String);
        auto Cached = Found != References.end() ? Found->second : nullptr;
        ReleaseSRWLockShared(&Lock);

        if (Cached)
            return Cached;

        auto Size = (wcslen(String) + 1) * sizeof(wchar_t);

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + Size <= End; Address += sizeof(wchar_t))
            {
                if (*(wchar_t*)Address == *String && !memcmp(Address, String, Size))
                {
                    if (auto Reference = FindReference(Address))
                    {
                        AcquireSRWLockExclusive(&Lock);
                        References.emplace(String, Reference);
                        ReleaseSRWLockExclusive(&Lock);

                        UE_LOG(LogVelora, Log, TEXT("Found \"%s\" at 0x%llX"), String, Reference - Base());
                        return Reference;
                    }
                }
            }
        }

        UE_LOG(LogVelora, Error, TEXT("Could not find \"%s\""), String);
        return nullptr;
    }

    inline const uint8_t* FindStringReference(const char* String)
    {
        static SRWLOCK Lock = SRWLOCK_INIT;
        static std::unordered_map<std::string, const uint8_t*> References;

        AcquireSRWLockShared(&Lock);
        auto Found = References.find(String);
        auto Cached = Found != References.end() ? Found->second : nullptr;
        ReleaseSRWLockShared(&Lock);

        if (Cached)
            return Cached;

        auto Size = strlen(String) + 1;

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + Size <= End; Address++)
            {
                if (*(char*)Address == *String && !memcmp(Address, String, Size))
                {
                    if (auto Reference = FindReference(Address))
                    {
                        AcquireSRWLockExclusive(&Lock);
                        References.emplace(String, Reference);
                        ReleaseSRWLockExclusive(&Lock);

                        UE_LOG(LogVelora, Log, TEXT("Found \"%hs\" at 0x%llX"), String, Reference - Base());
                        return Reference;
                    }
                }
            }
        }

        UE_LOG(LogVelora, Error, TEXT("Could not find \"%hs\""), String);
        return nullptr;
    }

    inline const uint8_t* FindPattern(const char* Pattern)
    {
        std::vector<int> Bytes;

        for (auto Character = Pattern; *Character;)
        {
            if (*Character == ' ')
                Character++;
            else if (*Character == '?')
            {
                Bytes.push_back(-1);
                Character++;
            }
            else
                Bytes.push_back((int)strtoul(Character, (char**)&Character, 16));
        }

        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + Bytes.size() <= End; Address++)
            {
                auto Index = 0ull;

                while (Index < Bytes.size() && (Bytes[Index] == -1 || Address[Index] == Bytes[Index]))
                    Index++;

                if (Index == Bytes.size())
                {
                    UE_LOG(LogVelora, Log, TEXT("Found pattern %hs at 0x%llX"), Pattern, Address - Base());
                    return Address;
                }
            }
        }

        UE_LOG(LogVelora, Error, TEXT("Could not find pattern %hs"), Pattern);
        return nullptr;
    }

    inline const RUNTIME_FUNCTION* FindFunction(const void* Address)
    {
        DWORD64 ImageBase = 0;
        const RUNTIME_FUNCTION* Function = RtlLookupFunctionEntry((DWORD64)Address, &ImageBase, nullptr);

        while (Function)
        {
            auto UnwindInfo = (const uint8_t*)ImageBase + Function->UnwindData;

            if (!((UnwindInfo[0] >> 3) & UNW_FLAG_CHAININFO))
                break;

            Function = (const RUNTIME_FUNCTION*)(UnwindInfo + 4 + ((UnwindInfo[2] + 1) & ~1) * sizeof(uint16_t));
        }

        return Function;
    }

    inline const uint8_t* FindFunctionEnd(const RUNTIME_FUNCTION* Function)
    {
        auto End = Base() + Function->EndAddress;
        DWORD64 ImageBase = 0;

        while (auto Chunk = RtlLookupFunctionEntry((DWORD64)End, &ImageBase, nullptr))
        {
            if (FindFunction(End)->BeginAddress != Function->BeginAddress)
                break;

            End = Base() + Chunk->EndAddress;
        }

        return End;
    }
}

namespace Scanner
{
    struct FInstruction
    {
        ZydisDecodedInstruction Info;
        ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT];
        char Text[256];
    };

    inline bool Decode(const uint8_t* Address, FInstruction& Instruction)
    {
        static auto Decoder = []
        {
            ZydisDecoder Decoder;
            ZydisDecoderInit(&Decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

            return Decoder;
        }();

        static auto Formatter = []
        {
            ZydisFormatter Formatter;
            ZydisFormatterInit(&Formatter, ZYDIS_FORMATTER_STYLE_INTEL);
            ZydisFormatterSetProperty(&Formatter, ZYDIS_FORMATTER_PROP_HEX_UPPERCASE, ZYAN_FALSE);
            ZydisFormatterSetProperty(&Formatter, ZYDIS_FORMATTER_PROP_ADDR_PADDING_ABSOLUTE, ZYDIS_PADDING_DISABLED);
            ZydisFormatterSetProperty(&Formatter, ZYDIS_FORMATTER_PROP_ADDR_PADDING_RELATIVE, ZYDIS_PADDING_DISABLED);
            ZydisFormatterSetProperty(&Formatter, ZYDIS_FORMATTER_PROP_DISP_PADDING, ZYDIS_PADDING_DISABLED);
            ZydisFormatterSetProperty(&Formatter, ZYDIS_FORMATTER_PROP_IMM_PADDING, ZYDIS_PADDING_DISABLED);

            return Formatter;
        }();

        if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(&Decoder, Address, ZYDIS_MAX_INSTRUCTION_LENGTH, &Instruction.Info, Instruction.Operands)))
            return false;

        return ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&Formatter, &Instruction.Info, Instruction.Operands, Instruction.Info.operand_count_visible, Instruction.Text, sizeof(Instruction.Text), (ZyanU64)Address, ZYAN_NULL));
    }

    inline bool Match(std::string_view Text, std::string_view Pattern)
    {
        size_t TextIndex = 0;
        size_t PatternIndex = 0;
        size_t Star = std::string_view::npos;
        size_t Resume = 0;

        while (TextIndex < Text.size())
        {
            if (PatternIndex < Pattern.size() && Pattern[PatternIndex] == '*')
            {
                Star = PatternIndex++;
                Resume = TextIndex;
            }
            else if (PatternIndex < Pattern.size() && (Pattern[PatternIndex] == '?' || Pattern[PatternIndex] == Text[TextIndex]))
            {
                PatternIndex++;
                TextIndex++;
            }
            else if (Star != std::string_view::npos)
            {
                PatternIndex = Star + 1;
                TextIndex = ++Resume;
            }
            else
            {
                return false;
            }
        }

        while (PatternIndex < Pattern.size() && Pattern[PatternIndex] == '*')
            PatternIndex++;

        return PatternIndex == Pattern.size();
    }

    inline bool Matches(const uint8_t* Address, std::string_view Pattern)
    {
        FInstruction Instruction;

        for (;;)
        {
            auto Separator = Pattern.find("; ");

            if (!Decode(Address, Instruction) || !Match(Instruction.Text, Pattern.substr(0, Separator)))
                return false;

            if (Separator == std::string_view::npos)
                return true;

            Address += Instruction.Info.length;
            Pattern.remove_prefix(Separator + 2);
        }
    }

    class FCode
    {
    public:
        FCode(const uint8_t* InAddress = nullptr, bool bInEntry = false)
            : Address(InAddress)
            , bEntry(bInEntry)
        {
        }

        FCode Find(std::string_view Pattern, int32 Index = 1) const
        {
            auto Found = Walk(bEntry, [&](const FWalkInstruction& Instruction)
            {
                return Matches(Instruction.Address, Pattern) && --Index == 0;
            });

            if (Address && !Found)
            {
                UE_LOG(LogVelora, Error, TEXT("Could not find \"%hs\" after 0x%llX"), std::string(Pattern).c_str(), Address - Base());
            }

            return FCode(Found);
        }

        FCode Last(std::string_view Pattern) const
        {
            const uint8_t* Found = nullptr;

            Start().Walk(true, [&](const FWalkInstruction& Instruction)
            {
                if (Matches(Instruction.Address, Pattern))
                    Found = Instruction.Address;

                return Instruction.Address + Instruction.Info.length >= Address;
            });

            if (Address && !Found)
            {
                UE_LOG(LogVelora, Error, TEXT("Could not find \"%hs\" before 0x%llX"), std::string(Pattern).c_str(), Address - Base());
            }

            return FCode(Found);
        }

        FCode Call(int32 Index = 1) const
        {
            auto Found = Walk(true, [&](const FInstruction& Instruction)
            {
                return Instruction.Info.mnemonic == ZYDIS_MNEMONIC_CALL && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE && --Index == 0;
            });

            if (Address && !Found)
            {
                UE_LOG(LogVelora, Error, TEXT("Could not find a call after 0x%llX"), Address - Base());
            }

            return FCode(FCode(Found).Target(), true);
        }

        FCode Start() const
        {
            auto Function = Address ? FindFunction(Address) : nullptr;

            return FCode(Function ? Base() + Function->BeginAddress : nullptr, true);
        }

        int32 Disp() const
        {
            FInstruction Instruction;

            if (Address && Decode(Address, Instruction))
            {
                for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count_visible; Index++)
                {
                    if (Instruction.Operands[Index].type == ZYDIS_OPERAND_TYPE_MEMORY)
                        return (int32)Instruction.Operands[Index].mem.disp.value;
                }
            }

            return 0;
        }

        int64 Imm() const
        {
            FInstruction Instruction;

            if (Address && Decode(Address, Instruction))
            {
                for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count_visible; Index++)
                {
                    if (Instruction.Operands[Index].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                        return Instruction.Operands[Index].imm.value.s;
                }
            }

            return 0;
        }

        const uint8_t* Target() const
        {
            FInstruction Instruction;

            if (Address && Decode(Address, Instruction))
            {
                for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count_visible; Index++)
                {
                    auto& Operand = Instruction.Operands[Index];
                    ZyanU64 Result;

                    if ((Operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Operand.imm.is_relative) || (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && Operand.mem.base == ZYDIS_REGISTER_RIP))
                    {
                        if (ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Operand, (ZyanU64)Address, &Result)))
                            return (const uint8_t*)Result;
                    }
                }
            }

            return nullptr;
        }

        bool Is(std::string_view Pattern) const
        {
            return Address && Matches(Address, Pattern);
        }

        bool Has(std::string_view Pattern) const
        {
            return Walk(bEntry, [&](const FWalkInstruction& Instruction)
            {
                return Matches(Instruction.Address, Pattern);
            });
        }

        const uint8_t* Get() const
        {
            return Address;
        }

    private:
        struct FWalkInstruction : FInstruction
        {
            const uint8_t* Address;
        };

        template<typename PredicateType>
        const uint8_t* Walk(bool bInclusive, PredicateType Predicate) const
        {
            if (!Address)
                return nullptr;

            auto Function = FindFunction(Address);
            auto End = Function ? FindFunctionEnd(Function) : nullptr;
            FWalkInstruction Instruction;

            for (Instruction.Address = Address; (!End || Instruction.Address < End) && Decode(Instruction.Address, Instruction); Instruction.Address += Instruction.Info.length)
            {
                if ((bInclusive || Instruction.Address != Address) && Predicate(Instruction))
                    return Instruction.Address;

                if (!End && Instruction.Info.mnemonic == ZYDIS_MNEMONIC_RET)
                    break;
            }

            return nullptr;
        }

        const uint8_t* Address;
        bool bEntry;
    };

    inline FCode String(const wchar_t* String)
    {
        auto Reference = FindStringReference(String);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        return FCode(Reference);
    }

    inline FCode String(const char* String)
    {
        auto Reference = FindStringReference(String);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        return FCode(Reference);
    }

    inline FCode At(const void* Address)
    {
        return FCode((const uint8_t*)Address, true);
    }

    inline FCode Pattern(const char* Signature)
    {
        return FCode(FindPattern(Signature), true);
    }

    inline FCode CallTo(const void* Function)
    {
        return FCode(FindCallReference(Function));
    }

    inline FCode Reference(const void* Target, std::string_view Pattern)
    {
        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin + ZYDIS_MAX_INSTRUCTION_LENGTH; Address + 4 <= End; Address++)
            {
                if (Address + 4 + *(int32_t*)Address != Target)
                    continue;

                for (auto Instruction = Address - 1; Instruction > Address - ZYDIS_MAX_INSTRUCTION_LENGTH; Instruction--)
                {
                    FCode Code(Instruction);

                    if (Code.Target() == Target && Code.Is(Pattern))
                    {
                        UE_LOG(LogVelora, Log, TEXT("Found \"%hs\" for 0x%llX at 0x%llX"), std::string(Pattern).c_str(), (const uint8_t*)Target - Base(), Instruction - Base());
                        return Code;
                    }
                }
            }
        }

        UE_LOG(LogVelora, Error, TEXT("Could not find \"%hs\" for 0x%llX"), std::string(Pattern).c_str(), (const uint8_t*)Target - Base());
        return FCode();
    }

    inline FCode Event(const TCHAR* FunctionName)
    {
        return Reference(String(FunctionName).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start();
    }

    inline FCode Exec(const TCHAR* FunctionPath)
    {
        return FCode(std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, FunctionPath))->GetNativeFunc()), true);
    }

    inline FCode Native(const TCHAR* FunctionPath)
    {
        auto Exec = Scanner::Exec(FunctionPath);

        for (auto Finish = Exec.Find("mov [*+0x20], *"); Finish.Get(); Finish = Finish.Find("mov [*+0x20], *"))
        {
            if (!Finish.Is("mov [rsp*"))
                return Finish.Call();
        }

        return FCode();
    }
}
