#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdlib>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>
#include <Zydis.h>
#include <Anchor/Anchor.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace Anchor
{
    static FHost Host;

    void SetHost(const FHost& InHost)
    {
        Host = InHost;
    }

    void Private::Log(ELogLevel Level, const wchar_t* Format, ...)
    {
        if (!Host.Log)
            return;

        wchar_t Message[1024];
        va_list Arguments;

        va_start(Arguments, Format);
        _vsnwprintf_s(Message, _TRUNCATE, Format, Arguments);
        va_end(Arguments);

        Host.Log(Level, Message);
    }

    int64_t Private::LogSlot(const wchar_t* Name, int64_t Value)
    {
        if (IsSlot(Value))
            Log(ELogLevel::Log, L"Found %s vtable slot 0x%llX", Name, Value);
        else
            Log(ELogLevel::Error, L"Could not find %s", Name);

        return Value;
    }

    void** Private::FindVTable(const wchar_t* Name)
    {
        return Host.FindVTable && Name ? Host.FindVTable(Name) : nullptr;
    }

    static int64_t LogOffset(const wchar_t* Name, int64_t Value)
    {
        if (Value > 0)
            Private::Log(ELogLevel::Log, L"Found %s offset 0x%llX", Name, Value);
        else
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);

        return Value;
    }

    uint8_t* Base()
    {
        return (uint8_t*)GetModuleHandleW(nullptr);
    }

    static std::span<IMAGE_SECTION_HEADER> Sections(uint8_t* Module = Base())
    {
        auto Headers = (IMAGE_NT_HEADERS*)(Module + ((IMAGE_DOS_HEADER*)Module)->e_lfanew);

        return { IMAGE_FIRST_SECTION(Headers), Headers->FileHeader.NumberOfSections };
    }

    bool IsCode(const void* Address)
    {
        for (auto Module : { Base(), (uint8_t*)&__ImageBase })
        {
            for (auto& Section : Sections(Module))
            {
                auto Begin = Module + Section.VirtualAddress;

                if ((Section.Characteristics & IMAGE_SCN_MEM_EXECUTE) && Address >= Begin && Address < Begin + Section.Misc.VirtualSize)
                    return true;
            }
        }

        return false;
    }

    static SRWLOCK WrittenLock = SRWLOCK_INIT;

    static std::vector<std::pair<const uint8_t*, const uint8_t*>>& WrittenRanges()
    {
        static std::vector<std::pair<const uint8_t*, const uint8_t*>> Ranges;

        return Ranges;
    }

    void Private::Written(const void* Address, size_t Size)
    {
        AcquireSRWLockExclusive(&WrittenLock);
        WrittenRanges().emplace_back((const uint8_t*)Address, (const uint8_t*)Address + Size);
        ReleaseSRWLockExclusive(&WrittenLock);
    }

    static uint64_t ImageSize()
    {
        return ((IMAGE_NT_HEADERS*)(Base() + ((IMAGE_DOS_HEADER*)Base())->e_lfanew))->OptionalHeader.SizeOfImage;
    }

    static bool InCode(const uint8_t* Address, int64_t Front, int64_t Back)
    {
        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;

            if ((Section.Characteristics & IMAGE_SCN_MEM_EXECUTE) && Address >= Begin + Front && Address + Back <= Begin + Section.Misc.VirtualSize)
                return true;
        }

        return false;
    }

    static const std::array<std::vector<uint64_t>, 3>& References()
    {
        static auto Tables = []
        {
            std::array<std::vector<uint64_t>, 3> Tables;
            std::vector<std::pair<uint64_t, uint64_t>> Code;
            auto Image = Base();
            auto Size = ImageSize();

            for (auto& Section : Sections())
            {
                if (Section.Characteristics & IMAGE_SCN_MEM_EXECUTE)
                    Code.emplace_back(Section.VirtualAddress, Section.VirtualAddress + Section.Misc.VirtualSize);
            }

            for (auto [CodeBegin, CodeEnd] : Code)
            {
                auto Begin = Image + CodeBegin;
                auto End = Image + CodeEnd;

                for (auto Address = Begin; Address + 4 <= End; Address++)
                {
                    auto Target = (uint64_t)(Address + 4 + *(int32_t*)Address - Image);

                    if (Target >= Size)
                        continue;

                    auto Key = Target << 32 | (uint64_t)(Address - Image);

                    if (Address >= Begin + 1 && (Address[-1] == 0xE8 || Address[-1] == 0xE9))
                        Tables[0].push_back(Key - 1);

                    if (Address >= Begin + 2 && Address[-2] == 0x8D && (Address[-1] & 0xC7) == 0x05)
                        Tables[1].push_back(Key - 2);

                    if (Address >= Begin + ZYDIS_MAX_INSTRUCTION_LENGTH && std::none_of(Code.begin(), Code.end(), [Target](auto& Range) { return Target >= Range.first && Target < Range.second; }))
                        Tables[2].push_back(Key);
                }
            }

            for (auto& Table : Tables)
                std::sort(Table.begin(), Table.end());

            return Tables;
        }();

        return Tables;
    }

    template<typename PredicateType>
    static const uint8_t* FindIndexed(int32_t Kind, const void* Target, PredicateType Predicate)
    {
        auto Image = Base();
        auto& Table = References()[Kind];
        auto Key = (uint64_t)((const uint8_t*)Target - Image) << 32;
        const uint8_t* Found = nullptr;

        for (auto Entry = std::lower_bound(Table.begin(), Table.end(), Key); Entry != Table.end() && (*Entry >> 32) == (Key >> 32); Entry++)
        {
            if (Predicate(Image + (uint32_t)*Entry))
            {
                Found = Image + (uint32_t)*Entry;
                break;
            }
        }

        AcquireSRWLockShared(&WrittenLock);

        for (auto [Begin, End] : WrittenRanges())
        {
            for (auto Address = Begin - ZYDIS_MAX_INSTRUCTION_LENGTH; Address < End && (!Found || Address < Found); Address++)
            {
                if (Predicate(Address))
                {
                    Found = Address;
                    break;
                }
            }
        }

        ReleaseSRWLockShared(&WrittenLock);

        return Found;
    }

    static const uint8_t* FindReference(const void* Target)
    {
        if ((uint64_t)((const uint8_t*)Target - Base()) < ImageSize())
            return FindIndexed(1, Target, [Target](const uint8_t* Address) { return InCode(Address, 0, 6) && Address[0] == 0x8D && (Address[1] & 0xC7) == 0x05 && Address + 6 + *(int32_t*)(Address + 2) == Target; });

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

    static const uint8_t* FindCallReference(const void* Target)
    {
        if ((uint64_t)((const uint8_t*)Target - Base()) < ImageSize())
            return FindIndexed(0, Target, [Target](const uint8_t* Address) { return InCode(Address, 0, 5) && (Address[0] == 0xE8 || Address[0] == 0xE9) && Address + 5 + *(int32_t*)(Address + 1) == Target; });

        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + 5 <= End; Address++)
            {
                if ((Address[0] == 0xE8 || Address[0] == 0xE9) && Address + 5 + *(int32_t*)(Address + 1) == Target)
                    return Address;
            }
        }

        return nullptr;
    }

    template<typename CharType>
    static const uint8_t* FindString(const CharType* Text, size_t Size)
    {
        auto Image = Base();
        auto& Table = References()[1];
        const uint8_t* Found = nullptr;

        auto IsText = [&](const uint8_t* Address)
        {
            if ((uint64_t)(Address - Image) >= ImageSize() || (Address - Image) % sizeof(CharType) || *(const CharType*)Address != *Text)
                return false;

            for (auto& Section : Sections())
            {
                auto Begin = Image + Section.VirtualAddress;

                if (Address >= Begin && Address + Size <= Begin + Section.Misc.VirtualSize)
                    return !memcmp(Address, Text, Size);
            }

            return false;
        };

        for (auto Entry = Table.begin(); Entry != Table.end() && !Found; Entry++)
        {
            if ((Entry == Table.begin() || (Entry[-1] >> 32) != (*Entry >> 32)) && IsText(Image + (*Entry >> 32)) && FindReference(Image + (*Entry >> 32)))
                Found = Image + (*Entry >> 32);
        }

        AcquireSRWLockShared(&WrittenLock);

        for (auto [Begin, End] : WrittenRanges())
        {
            for (auto Address = Begin - ZYDIS_MAX_INSTRUCTION_LENGTH; Address < End; Address++)
            {
                if (InCode(Address, 0, 6) && Address[0] == 0x8D && (Address[1] & 0xC7) == 0x05)
                {
                    auto Target = Address + 6 + *(int32_t*)(Address + 2);

                    if ((!Found || Target < Found) && IsText(Target))
                        Found = Target;
                }
            }
        }

        ReleaseSRWLockShared(&WrittenLock);

        return Found ? FindReference(Found) : nullptr;
    }

    static const uint8_t* FindStringReference(const wchar_t* Text)
    {
        static SRWLOCK Lock = SRWLOCK_INIT;
        static std::unordered_map<std::wstring, const uint8_t*> References;

        AcquireSRWLockShared(&Lock);
        auto Found = References.find(Text);
        auto Cached = Found != References.end() ? Found->second : nullptr;
        ReleaseSRWLockShared(&Lock);

        if (Cached)
            return Cached;

        auto Length = wcslen(Text);
        auto Reference = FindString(Text, (Text[Length - 1] == L'*' ? Length - 1 : Length + 1) * sizeof(wchar_t));

        if (Reference)
        {
            AcquireSRWLockExclusive(&Lock);
            References.emplace(Text, Reference);
            ReleaseSRWLockExclusive(&Lock);
        }

        return Reference;
    }

    static const uint8_t* FindStringReference(const char* Text)
    {
        static SRWLOCK Lock = SRWLOCK_INIT;
        static std::unordered_map<std::string, const uint8_t*> References;

        AcquireSRWLockShared(&Lock);
        auto Found = References.find(Text);
        auto Cached = Found != References.end() ? Found->second : nullptr;
        ReleaseSRWLockShared(&Lock);

        if (Cached)
            return Cached;

        auto Length = strlen(Text);
        auto Reference = FindString(Text, Text[Length - 1] == '*' ? Length - 1 : Length + 1);

        if (Reference)
        {
            AcquireSRWLockExclusive(&Lock);
            References.emplace(Text, Reference);
            ReleaseSRWLockExclusive(&Lock);
        }

        return Reference;
    }

    static const uint8_t* FindPattern(const char* Signature)
    {
        std::vector<int> Bytes;

        for (auto Character = Signature; *Character;)
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
                    return Address;
            }
        }

        return nullptr;
    }

    static const RUNTIME_FUNCTION* FindFunction(const void* Address)
    {
        DWORD64 ImageBase = 0;
        const RUNTIME_FUNCTION* Entry = RtlLookupFunctionEntry((DWORD64)Address, &ImageBase, nullptr);

        while (Entry)
        {
            auto UnwindInfo = (const uint8_t*)ImageBase + Entry->UnwindData;

            if (!((UnwindInfo[0] >> 3) & UNW_FLAG_CHAININFO))
                break;

            Entry = (const RUNTIME_FUNCTION*)(UnwindInfo + 4 + ((UnwindInfo[2] + 1) & ~1) * sizeof(uint16_t));
        }

        return Entry;
    }

    static const uint8_t* FindFunctionEnd(const RUNTIME_FUNCTION* Entry)
    {
        auto End = Base() + Entry->EndAddress;
        DWORD64 ImageBase = 0;

        while (auto Chunk = RtlLookupFunctionEntry((DWORD64)End, &ImageBase, nullptr))
        {
            if (FindFunction(End)->BeginAddress != Entry->BeginAddress)
                break;

            End = Base() + Chunk->EndAddress;
        }

        return End;
    }

    struct FInstruction
    {
        ZydisDecodedInstruction Info;
        ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT];
        char Text[256];
        const uint8_t* Address;
    };

    static bool Decode(const uint8_t* Address, FInstruction& Instruction)
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

    static bool Match(std::string_view Text, std::string_view Pattern)
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

    static bool Matches(const uint8_t* Address, std::string_view Instructions)
    {
        FInstruction Instruction;

        for (;;)
        {
            auto Separator = Instructions.find("; ");

            if (!Decode(Address, Instruction) || !Match(Instruction.Text, Instructions.substr(0, Separator)))
                return false;

            if (Separator == std::string_view::npos)
                return true;

            Address += Instruction.Info.length;
            Instructions.remove_prefix(Separator + 2);
        }
    }

    template<typename PredicateType>
    static const uint8_t* Walk(const uint8_t* Address, bool bInclusive, PredicateType Predicate)
    {
        if (!Address)
            return nullptr;

        auto Entry = FindFunction(Address);
        auto End = Entry ? FindFunctionEnd(Entry) : nullptr;
        FInstruction Instruction;

        for (Instruction.Address = Address; !End || Instruction.Address < End; Instruction.Address += Instruction.Info.length)
        {
            if (!Decode(Instruction.Address, Instruction))
            {
                if (!End)
                    break;

                Instruction.Info.length = 1;
                continue;
            }

            if ((bInclusive || Instruction.Address != Address) && Predicate(Instruction))
                return Instruction.Address;

            if (!End && Instruction.Info.mnemonic == ZYDIS_MNEMONIC_RET)
                break;
        }

        return nullptr;
    }

    FCode FCode::Find(std::string_view Instructions, int32_t Index) const
    {
        return FCode(Walk(Address, bEntry, [&](const FInstruction& Instruction)
        {
            return Matches(Instruction.Address, Instructions) && --Index == 0;
        }));
    }

    FCode FCode::Last(std::string_view Instructions) const
    {
        const uint8_t* Found = nullptr;

        Walk(Start().Address, true, [&](const FInstruction& Instruction)
        {
            if (Matches(Instruction.Address, Instructions))
                Found = Instruction.Address;

            return Instruction.Address + Instruction.Info.length >= Address;
        });

        return FCode(Found);
    }

    FCode FCode::Call(int32_t Index) const
    {
        auto Found = Walk(Address, true, [&](const FInstruction& Instruction)
        {
            return Instruction.Info.mnemonic == ZYDIS_MNEMONIC_CALL && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE && --Index == 0;
        });

        return FCode(FCode(Found).Target(), true);
    }

    FCode FCode::Start() const
    {
        auto Entry = Address ? FindFunction(Address) : nullptr;

        return FCode(Entry ? Base() + Entry->BeginAddress : nullptr, true);
    }

    int32_t FCode::Disp() const
    {
        FInstruction Instruction;

        if (Address && Decode(Address, Instruction))
        {
            for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count_visible; Index++)
            {
                if (Instruction.Operands[Index].type == ZYDIS_OPERAND_TYPE_MEMORY)
                    return (int32_t)Instruction.Operands[Index].mem.disp.value;
            }
        }

        return 0;
    }

    int32_t FCode::Disp(const wchar_t* Name) const
    {
        return (int32_t)LogOffset(Name, Disp());
    }

    int32_t FCode::Slot(const wchar_t* Name) const
    {
        return (int32_t)Private::LogSlot(Name, Disp());
    }

    int64_t FCode::Imm() const
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

    int64_t FCode::Imm(const wchar_t* Name) const
    {
        return LogOffset(Name, Imm());
    }

    const uint8_t* FCode::Target() const
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
                        return Private::Unhooked((const uint8_t*)Result);
                }
            }
        }

        return nullptr;
    }

    const uint8_t* FCode::Target(const wchar_t* Name) const
    {
        return FCode(Target()).Get(Name);
    }

    bool FCode::Is(std::string_view Instructions) const
    {
        return Address && Matches(Address, Instructions);
    }

    bool FCode::Has(std::string_view Instructions) const
    {
        return Walk(Address, bEntry, [&](const FInstruction& Instruction)
        {
            return Matches(Instruction.Address, Instructions);
        });
    }

    const uint8_t* FCode::Get(const wchar_t* Name) const
    {
        if (Address)
            Private::Log(ELogLevel::Log, L"Found %s at 0x%llX", Name, Address - Base());
        else
            Private::Log(ELogLevel::Error, L"Could not find %s", Name);

        return Address;
    }

    FCode String(const wchar_t* Text)
    {
        auto Reference = FindStringReference(Text);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        return FCode(Reference);
    }

    FCode String(const char* Text)
    {
        auto Reference = FindStringReference(Text);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        return FCode(Reference);
    }

    FCode Smallest(const wchar_t* Text)
    {
        auto Size = (wcslen(Text) + 1) * sizeof(wchar_t);
        const uint8_t* Found = nullptr;
        int64_t FoundSize = 0;

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + Size <= End; Address += sizeof(wchar_t))
            {
                if (*(wchar_t*)Address != *Text || memcmp(Address, Text, Size))
                    continue;

                for (auto& CodeSection : Sections())
                {
                    if (!(CodeSection.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                        continue;

                    auto CodeBegin = Base() + CodeSection.VirtualAddress;
                    auto CodeEnd = CodeBegin + CodeSection.Misc.VirtualSize;

                    for (auto Reference = CodeBegin + 1; Reference + 6 <= CodeEnd; Reference++)
                    {
                        if (Reference[0] != 0x8D || (Reference[1] & 0xC7) != 0x05 || Reference + 6 + *(int32_t*)(Reference + 2) != Address)
                            continue;

                        auto Entry = FindFunction(Reference);

                        if (!Entry)
                            continue;

                        auto EntrySize = FindFunctionEnd(Entry) - (Base() + Entry->BeginAddress);

                        if (!Found || EntrySize < FoundSize)
                        {
                            Found = (Reference[-1] & 0xFB) == 0x48 ? Reference - 1 : Reference;
                            FoundSize = EntrySize;
                        }
                    }
                }
            }
        }

        return FCode(Found);
    }

    FCode Pattern(const char* Signature)
    {
        return FCode(FindPattern(Signature), true);
    }

    FCode CallTo(const void* Function)
    {
        return FCode(Function ? FindCallReference(Function) : nullptr);
    }

    FCode Reference(const void* Target, std::string_view Instructions)
    {
        if (!Target)
            return FCode();

        auto Referenced = [&](const uint8_t* Address)
        {
            for (auto Instruction = Address - 1; Instruction > Address - ZYDIS_MAX_INSTRUCTION_LENGTH; Instruction--)
            {
                FCode Code(Instruction);

                if (Code.Target() == Target && Code.Is(Instructions))
                    return Code;
            }

            return FCode();
        };

        if ((uint64_t)((const uint8_t*)Target - Base()) < ImageSize() && !InCode((const uint8_t*)Target, 0, 1))
        {
            auto Found = FindIndexed(2, Target, [&](const uint8_t* Address) { return InCode(Address, ZYDIS_MAX_INSTRUCTION_LENGTH, 4) && Address + 4 + *(int32_t*)Address == Target && Referenced(Address).Get(); });

            return Found ? Referenced(Found) : FCode();
        }

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

                if (auto Code = Referenced(Address); Code.Get())
                    return Code;
            }
        }

        return FCode();
    }

    FCode String(const wchar_t* Text, std::string_view Instructions)
    {
        auto Size = (wcslen(Text) + 1) * sizeof(wchar_t);

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin; Address + Size <= End; Address += sizeof(wchar_t))
            {
                if (*(wchar_t*)Address == *Text && !memcmp(Address, Text, Size))
                {
                    if (auto Code = Reference(Address, Instructions); Code.Get())
                        return Code;
                }
            }
        }

        return FCode();
    }
}
