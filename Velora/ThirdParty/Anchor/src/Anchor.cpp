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

    static const std::array<std::vector<uint64_t>, 4>& References()
    {
        static auto Tables = []
        {
            std::array<std::vector<uint64_t>, 4> Tables;
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

                    if (Address >= Begin + 3 && Address[-3] == 0x0F && (Address[-2] == 0x10 || Address[-2] == 0x28) && (Address[-1] & 0xC7) == 0x05)
                        Tables[3].push_back(Key - 3);

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

    static const std::vector<uint64_t>& Pointers()
    {
        static auto Table = []
        {
            std::vector<uint64_t> Table;
            auto Image = Base();
            auto Size = ImageSize();

            for (auto Entry : References()[1])
            {
                auto Target = Entry >> 32;

                if (Target % sizeof(uint64_t))
                    continue;

                for (auto& Section : Sections())
                {
                    if ((Section.Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE)) || Target < Section.VirtualAddress || Target + sizeof(uint64_t) > Section.VirtualAddress + Section.Misc.VirtualSize)
                        continue;

                    auto Value = *(uint64_t*)(Image + Target) - (uint64_t)Image;

                    if (Value < Size)
                        Table.push_back(Value << 32 | Target);
                }
            }

            std::sort(Table.begin(), Table.end());
            Table.erase(std::unique(Table.begin(), Table.end()), Table.end());

            return Table;
        }();

        return Table;
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

        if (Found)
            return FindReference(Found);

        auto& LoadTable = References()[3];

        for (auto Entry = LoadTable.begin(); Entry != LoadTable.end() && !Found; Entry++)
        {
            if ((Entry == LoadTable.begin() || (Entry[-1] >> 32) != (*Entry >> 32)) && IsText(Image + (*Entry >> 32)))
                Found = Image + (*Entry >> 32);
        }

        if (Found)
            return FindIndexed(3, Found, [Found](const uint8_t* Address) { return InCode(Address, 0, 7) && Address[0] == 0x0F && (Address[1] == 0x10 || Address[1] == 0x28) && (Address[2] & 0xC7) == 0x05 && Address + 7 + *(int32_t*)(Address + 3) == Found; });

        for (auto Entry : Pointers())
        {
            if (IsText(Image + (Entry >> 32)))
                return FindReference(Image + (uint32_t)Entry);
        }

        return nullptr;
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

    static std::pair<const uint8_t*, const uint8_t*> FindRange(const void* Address)
    {
        auto Entry = Address ? FindFunction(Address) : nullptr;

        if (!Entry)
            return { nullptr, nullptr };

        auto Begin = Base() + Entry->BeginAddress;
        auto End = FindFunctionEnd(Entry);

        if (Address >= Begin && Address < End)
            return { Begin, End };

        DWORD64 ImageBase = 0;
        auto Chunk = RtlLookupFunctionEntry((DWORD64)Address, &ImageBase, nullptr);
        Begin = Base() + Chunk->BeginAddress;
        End = Base() + Chunk->EndAddress;

        while (auto Previous = RtlLookupFunctionEntry((DWORD64)(Begin - 1), &ImageBase, nullptr))
        {
            if (FindFunction(Begin - 1)->BeginAddress != Entry->BeginAddress)
                break;

            Begin = Base() + Previous->BeginAddress;
        }

        while (auto Next = RtlLookupFunctionEntry((DWORD64)End, &ImageBase, nullptr))
        {
            if (FindFunction(End)->BeginAddress != Entry->BeginAddress)
                break;

            End = Base() + Next->EndAddress;
        }

        return { Begin, End };
    }

    std::pair<const uint8_t*, const uint8_t*> Range(const void* Address)
    {
        return FindRange(Address);
    }

    struct FInstruction
    {
        ZydisDecodedInstruction Info;
        ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT];
        char Text[256];
        const uint8_t* Address;
    };

    static bool Decode(const uint8_t* Address, FInstruction& Instruction, bool bFormat = true)
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

        return !bFormat || ZYAN_SUCCESS(ZydisFormatterFormatInstruction(&Formatter, &Instruction.Info, Instruction.Operands, Instruction.Info.operand_count_visible, Instruction.Text, sizeof(Instruction.Text), (ZyanU64)Address, ZYAN_NULL));
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

        auto End = FindRange(Address).second;
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

        Walk(FindRange(Address).first, true, [&](const FInstruction& Instruction)
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

    static const uint8_t* FindLogCaller(const uint8_t* Reference)
    {
        auto Entry = Reference ? FindFunction(Reference) : nullptr;

        if (!Entry || Reference < Base() + Entry->BeginAddress)
            return Reference;

        auto Begin = Base() + Entry->BeginAddress;
        bool bClearsFile = false;
        bool bClearsLine = false;
        bool bPassesFile = false;
        bool bPassesLine = false;
        bool bPassesVerbosity = false;
        FInstruction Instruction;

        for (auto Address = Begin; Address < Base() + Entry->EndAddress && Decode(Address, Instruction); Address += Instruction.Info.length)
        {
            if (Instruction.Info.meta.category == ZYDIS_CATEGORY_COND_BR || Instruction.Info.meta.category == ZYDIS_CATEGORY_UNCOND_BR || Instruction.Info.mnemonic == ZYDIS_MNEMONIC_RET)
                break;

            bClearsFile |= Match(Instruction.Text, "xor ecx, ecx");
            bClearsLine |= Match(Instruction.Text, "xor edx, edx");
            bPassesFile |= Match(Instruction.Text, "lea rcx, [0x*]");
            bPassesLine |= Match(Instruction.Text, "mov edx, 0x*") || Match(Instruction.Text, "lea edx, [*]");
            bPassesVerbosity |= Match(Instruction.Text, "mov r9d, 0x*");

            if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_CALL && Address > Reference && ((bClearsFile && bClearsLine) || (bPassesFile && bPassesLine && bPassesVerbosity)))
            {
                auto Caller = FindCallReference(Begin);

                return Caller ? Caller : Reference;
            }
        }

        return Reference;
    }

    struct FEncryptedBuilder
    {
        const uint8_t* Begin = nullptr;
        const uint8_t* End = nullptr;
        const uint8_t* Site = nullptr;
        std::vector<int16_t> Cipher;
    };

    struct FEncryptedStrings
    {
        std::vector<std::array<uint8_t, 256>> Keystreams;
        std::vector<FEncryptedBuilder> Builders;
        std::vector<uint64_t> Stores;
        std::vector<uint32_t> Blobs;
    };

    static ZydisRegister LargestRegister(ZydisRegister Register)
    {
        return ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Register);
    }

    static uint64_t RegisterMask(uint32_t Bits)
    {
        return Bits >= 64 ? ~0ull : (1ull << Bits) - 1;
    }

    static bool IsGeneralRegister(ZydisRegister Register)
    {
        auto Class = ZydisRegisterGetClass(Register);

        return (Class == ZYDIS_REGCLASS_GPR8 || Class == ZYDIS_REGCLASS_GPR16 || Class == ZYDIS_REGCLASS_GPR32 || Class == ZYDIS_REGCLASS_GPR64) && Register != ZYDIS_REGISTER_AH && Register != ZYDIS_REGISTER_CH && Register != ZYDIS_REGISTER_DH && Register != ZYDIS_REGISTER_BH;
    }

    static bool WritesGeneralRegister(const FInstruction& Instruction, bool (&Written)[ZYDIS_REGISTER_MAX_VALUE + 1])
    {
        bool bWrites = false;

        for (uint8_t Index = 0; Index < Instruction.Info.operand_count; Index++)
        {
            auto& Operand = Instruction.Operands[Index];
            auto Class = Operand.type == ZYDIS_OPERAND_TYPE_REGISTER ? ZydisRegisterGetClass(Operand.reg.value) : ZYDIS_REGCLASS_INVALID;

            if ((Operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) && (Class == ZYDIS_REGCLASS_GPR8 || Class == ZYDIS_REGCLASS_GPR16 || Class == ZYDIS_REGCLASS_GPR32 || Class == ZYDIS_REGCLASS_GPR64))
            {
                Written[LargestRegister(Operand.reg.value)] = true;
                bWrites = true;
            }
        }

        return bWrites;
    }

    static bool IsKeyXor(const FInstruction& Instruction)
    {
        return Instruction.Info.mnemonic == ZYDIS_MNEMONIC_XOR && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY && Instruction.Operands[0].size == 8 && Instruction.Operands[1].type == ZYDIS_OPERAND_TYPE_REGISTER;
    }

    static bool RunKeystream(const std::vector<FInstruction>& Body, ZydisRegister Input, ZydisRegister Output, uint64_t Key, uint8_t& Result)
    {
        uint64_t Registers[ZYDIS_REGISTER_MAX_VALUE + 1] = {};
        bool Written[ZYDIS_REGISTER_MAX_VALUE + 1] = {};

        auto Read = [&](const ZydisDecodedOperand& Operand, uint64_t& Value)
        {
            if (Operand.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                Value = Operand.imm.value.u;
            else if (Operand.type == ZYDIS_OPERAND_TYPE_REGISTER && IsGeneralRegister(Operand.reg.value))
                Value = Registers[LargestRegister(Operand.reg.value)] & RegisterMask(Operand.size);
            else
                return false;

            return true;
        };

        if (!IsGeneralRegister(Input) || !IsGeneralRegister(Output))
            return false;

        Registers[LargestRegister(Input)] = Key;

        for (auto& Instruction : Body)
        {
            if (!WritesGeneralRegister(Instruction, Written))
                continue;

            auto& Destination = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];

            if (Destination.type != ZYDIS_OPERAND_TYPE_REGISTER || !IsGeneralRegister(Destination.reg.value))
                return false;

            auto& Slot = Registers[LargestRegister(Destination.reg.value)];
            auto Current = Slot & RegisterMask(Destination.size);
            auto Mnemonic = Instruction.Info.mnemonic;
            uint64_t Value = Current;
            uint64_t Other = 0;

            switch (Mnemonic)
            {
            case ZYDIS_MNEMONIC_MOV:
            case ZYDIS_MNEMONIC_MOVZX:
                if (!Read(Source, Value))
                    return false;

                break;
            case ZYDIS_MNEMONIC_LEA:
                if (Source.mem.base == ZYDIS_REGISTER_RIP)
                    return false;

                Value = Source.mem.disp.value;

                if (Source.mem.base != ZYDIS_REGISTER_NONE)
                    Value += Registers[LargestRegister(Source.mem.base)] & RegisterMask(ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, Source.mem.base));

                if (Source.mem.index != ZYDIS_REGISTER_NONE)
                    Value += (Registers[LargestRegister(Source.mem.index)] & RegisterMask(ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, Source.mem.index))) * Source.mem.scale;

                break;
            case ZYDIS_MNEMONIC_IMUL:
                if (Instruction.Info.operand_count_visible == 3 ? !Read(Source, Value) || !Read(Instruction.Operands[2], Other) : Instruction.Info.operand_count_visible != 2 || !Read(Source, Other))
                    return false;

                Value *= Other;
                break;
            case ZYDIS_MNEMONIC_ADD:
            case ZYDIS_MNEMONIC_SUB:
            case ZYDIS_MNEMONIC_XOR:
            case ZYDIS_MNEMONIC_AND:
            case ZYDIS_MNEMONIC_OR:
                if (!Read(Source, Other))
                    return false;

                Value = Mnemonic == ZYDIS_MNEMONIC_ADD ? Current + Other : Mnemonic == ZYDIS_MNEMONIC_SUB ? Current - Other : Mnemonic == ZYDIS_MNEMONIC_XOR ? Current ^ Other : Mnemonic == ZYDIS_MNEMONIC_AND ? Current & Other : Current | Other;
                break;
            case ZYDIS_MNEMONIC_NOT:
                Value = ~Current;
                break;
            case ZYDIS_MNEMONIC_NEG:
                Value = 0 - Current;
                break;
            case ZYDIS_MNEMONIC_INC:
                Value = Current + 1;
                break;
            case ZYDIS_MNEMONIC_DEC:
                Value = Current - 1;
                break;
            case ZYDIS_MNEMONIC_SHL:
            case ZYDIS_MNEMONIC_SHR:
            case ZYDIS_MNEMONIC_ROL:
            case ZYDIS_MNEMONIC_ROR:
            {
                if (!Read(Source, Other))
                    return false;

                auto Count = Other & (Destination.size == 64 ? 63 : 31);
                auto Rotate = Count % Destination.size;

                Value = Mnemonic == ZYDIS_MNEMONIC_SHL ? Current << Count : Mnemonic == ZYDIS_MNEMONIC_SHR ? Current >> Count : !Rotate ? Current : Mnemonic == ZYDIS_MNEMONIC_ROL ? Current << Rotate | Current >> (Destination.size - Rotate) : Current >> Rotate | Current << (Destination.size - Rotate);
                break;
            }
            default:
                return false;
            }

            Value &= RegisterMask(Destination.size);
            Slot = Destination.size >= 32 ? Value : (Slot & ~RegisterMask(Destination.size)) | Value;
        }

        Result = (uint8_t)Registers[LargestRegister(Output)];

        return true;
    }

    static bool FindKeystream(const uint8_t* Site, std::array<uint8_t, 256>& Keystream)
    {
        std::vector<FInstruction> Body;
        FInstruction Instruction;
        auto Input = ZYDIS_REGISTER_NONE;
        auto Address = Site + 5;
        bool bLooped = false;

        for (int32_t Count = 0; Count < 128; Count++)
        {
            if (!InCode(Address, 0, ZYDIS_MAX_INSTRUCTION_LENGTH) || !Decode(Address, Instruction, false))
                return false;

            auto Category = Instruction.Info.meta.category;

            if (IsKeyXor(Instruction) && Input != ZYDIS_REGISTER_NONE)
            {
                for (int32_t Key = 0; Key < 256; Key++)
                {
                    uint8_t Low = 0;
                    uint8_t High = 0;

                    if (!RunKeystream(Body, Input, Instruction.Operands[1].reg.value, Key, Low) || !RunKeystream(Body, Input, Instruction.Operands[1].reg.value, Key | 0x5A5A00, High) || Low != High)
                        return false;

                    Keystream[Key] = Low;
                }

                return std::any_of(Keystream.begin(), Keystream.end(), [&](uint8_t Value) { return Value != Keystream[0]; });
            }

            if (IsKeyXor(Instruction))
            {
                Input = Instruction.Operands[1].reg.value;
                Address += Instruction.Info.length;
                continue;
            }

            if (Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET || Category == ZYDIS_CATEGORY_UNCOND_BR)
                return false;

            ZyanU64 Target = 0;

            if (Category == ZYDIS_CATEGORY_COND_BR && Input != ZYDIS_REGISTER_NONE && !bLooped && ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Instruction.Operands[0], (ZyanU64)Address, &Target)) && (const uint8_t*)Target < Address)
            {
                bLooped = true;
                Address = (const uint8_t*)Target;
                continue;
            }

            if (Input != ZYDIS_REGISTER_NONE)
                Body.push_back(Instruction);

            Address += Instruction.Info.length;
        }

        return false;
    }

    static bool ReadBuilder(const uint8_t* Address, FEncryptedBuilder& Builder)
    {
        FInstruction Instruction;
        bool bReturnsBuffer = false;

        Builder.Begin = Address;
        Builder.Cipher.clear();

        for (int32_t Count = 0; Count < 1024 && InCode(Address, 0, ZYDIS_MAX_INSTRUCTION_LENGTH) && Decode(Address, Instruction, false); Count++, Address += Instruction.Info.length)
        {
            auto& Destination = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];

            if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_RET)
            {
                Builder.End = Address + Instruction.Info.length;

                return bReturnsBuffer && !Builder.Cipher.empty() && std::find(Builder.Cipher.begin(), Builder.Cipher.end(), -1) == Builder.Cipher.end();
            }

            if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_MOV && Destination.type == ZYDIS_OPERAND_TYPE_MEMORY && Destination.mem.base == ZYDIS_REGISTER_RDX && Destination.mem.index == ZYDIS_REGISTER_NONE && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Destination.mem.disp.value >= 0 && Destination.mem.disp.value < 0x1000)
            {
                auto Offset = (size_t)Destination.mem.disp.value;
                auto Size = (size_t)Destination.size / 8;

                if (Builder.Cipher.size() < Offset + Size)
                    Builder.Cipher.resize(Offset + Size, -1);

                for (size_t Index = 0; Index < Size; Index++)
                {
                    if (Builder.Cipher[Offset + Index] != -1)
                        return false;

                    Builder.Cipher[Offset + Index] = (uint8_t)(Source.imm.value.u >> (Index * 8));
                }

                continue;
            }

            auto Mnemonic = Instruction.Info.mnemonic;
            auto bMove = Mnemonic == ZYDIS_MNEMONIC_MOV;
            auto bRegisters = Destination.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.type == ZYDIS_OPERAND_TYPE_REGISTER;
            auto bReturns = bMove && bRegisters && Destination.reg.value == ZYDIS_REGISTER_RAX && Source.reg.value == ZYDIS_REGISTER_RDX;
            auto bFrame = bMove && bRegisters && Destination.reg.value == ZYDIS_REGISTER_R11 && Source.reg.value == ZYDIS_REGISTER_RSP;
            auto bSave = bMove && Destination.type == ZYDIS_OPERAND_TYPE_MEMORY && Source.type == ZYDIS_OPERAND_TYPE_REGISTER && (Destination.mem.base == ZYDIS_REGISTER_RSP || Destination.mem.base == ZYDIS_REGISTER_R11);
            auto bRestore = bMove && Destination.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.type == ZYDIS_OPERAND_TYPE_MEMORY && (Source.mem.base == ZYDIS_REGISTER_RSP || Source.mem.base == ZYDIS_REGISTER_R11);
            auto bStack = (Mnemonic == ZYDIS_MNEMONIC_SUB || Mnemonic == ZYDIS_MNEMONIC_ADD) && Destination.type == ZYDIS_OPERAND_TYPE_REGISTER && Destination.reg.value == ZYDIS_REGISTER_RSP;

            if (!bReturns && !bFrame && !bSave && !bRestore && !bStack && Mnemonic != ZYDIS_MNEMONIC_PUSH && Mnemonic != ZYDIS_MNEMONIC_POP)
                return false;

            bReturnsBuffer |= bReturns;
        }

        return false;
    }

    static bool ReadStores(const uint8_t* Address, size_t Size, std::vector<int16_t>& Cipher)
    {
        FInstruction Instruction;

        if (!Decode(Address, Instruction, false) || Instruction.Info.mnemonic != ZYDIS_MNEMONIC_MOV || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY || Instruction.Operands[1].type != ZYDIS_OPERAND_TYPE_IMMEDIATE || Instruction.Operands[0].mem.index != ZYDIS_REGISTER_NONE)
            return false;

        auto Buffer = Instruction.Operands[0].mem.base;
        auto Origin = Instruction.Operands[0].mem.disp.value;
        uint64_t Values[ZYDIS_REGISTER_MAX_VALUE + 1] = {};
        bool Known[ZYDIS_REGISTER_MAX_VALUE + 1] = {};

        auto Missing = Size;

        Cipher.assign(Size, -2);

        for (int32_t Count = 0; Count < 512 && Missing && Decode(Address, Instruction, false); Count++, Address += Instruction.Info.length)
        {
            auto Category = Instruction.Info.meta.category;
            auto& Destination = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];

            if (Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET || Category == ZYDIS_CATEGORY_COND_BR || Category == ZYDIS_CATEGORY_UNCOND_BR)
                break;

            if (Destination.type == ZYDIS_OPERAND_TYPE_MEMORY && Destination.mem.base == Buffer)
            {
                if (Instruction.Info.mnemonic != ZYDIS_MNEMONIC_MOV || Destination.mem.index != ZYDIS_REGISTER_NONE)
                    break;

                auto bKnown = Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE || (Source.type == ZYDIS_OPERAND_TYPE_REGISTER && IsGeneralRegister(Source.reg.value) && Known[LargestRegister(Source.reg.value)]);
                auto Value = Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE ? Source.imm.value.u : bKnown ? Values[LargestRegister(Source.reg.value)] : 0;

                for (int64_t Index = 0; Index < Destination.size / 8; Index++)
                {
                    auto Offset = Destination.mem.disp.value + Index - Origin;

                    if (Offset < 0 || Offset >= (int64_t)Size)
                        continue;

                    Missing -= Cipher[Offset] == -2;
                    Cipher[Offset] = bKnown ? (uint8_t)(Value >> (Index * 8)) : -1;
                }

                continue;
            }

            if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_MOV && Destination.type == ZYDIS_OPERAND_TYPE_REGISTER && IsGeneralRegister(Destination.reg.value) && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                auto Register = LargestRegister(Destination.reg.value);
                auto Value = Source.imm.value.u & RegisterMask(Destination.size);

                Values[Register] = Destination.size >= 32 ? Value : (Values[Register] & ~RegisterMask(Destination.size)) | Value;
                Known[Register] = Destination.size >= 32 || Known[Register];
                continue;
            }

            bool Written[ZYDIS_REGISTER_MAX_VALUE + 1] = {};

            if (!WritesGeneralRegister(Instruction, Written))
                continue;

            for (size_t Register = 0; Register <= ZYDIS_REGISTER_MAX_VALUE; Register++)
                Known[Register] = Known[Register] && !Written[Register];
        }

        return !Missing;
    }

    template<typename ByteType>
    static bool Decrypts(const std::array<uint8_t, 256>& Keystream, const ByteType* Cipher, std::span<const uint8_t> Plain)
    {
        auto Key = (uint8_t)(Cipher[0] ^ Plain[0]);

        for (size_t Index = 1; Index < Plain.size(); Index++)
        {
            Key = Keystream[Key];

            if (Cipher[Index] >= 0 && (uint8_t)(Cipher[Index] ^ Key) != Plain[Index])
                return false;
        }

        return Cipher[0] >= 0;
    }

    static const FEncryptedStrings& EncryptedStrings()
    {
        static auto Strings = []
        {
            FEncryptedStrings Strings;
            std::vector<std::pair<std::array<uint8_t, 256>, int32_t>> Counts;
            auto Image = Base();
            auto& Calls = References()[0];
            std::vector<const uint8_t*> Frequent;
            int32_t Found = 0;

            auto IsCall = [&](uint64_t Call)
            {
                return Image[(uint32_t)Call] == 0xE8 && InCode(Image + (uint32_t)Call, 0, 5);
            };

            auto Sample = [&](const uint8_t* Site)
            {
                std::array<uint8_t, 256> Keystream;

                if (Found >= 64 || !FindKeystream(Site, Keystream))
                    return;

                auto Count = std::find_if(Counts.begin(), Counts.end(), [&](const auto& Pair) { return Pair.first == Keystream; });

                if (Count != Counts.end())
                    Count->second++;
                else
                    Counts.emplace_back(Keystream, 1);

                Found++;
            };

            for (auto Entry = Calls.begin(); Entry != Calls.end();)
            {
                auto Last = std::upper_bound(Entry, Calls.end(), *Entry | 0xFFFFFFFF);
                auto Site = std::find_if(Entry, Last, IsCall);
                FEncryptedBuilder Builder;

                if (Site != Last && InCode(Image + (*Entry >> 32), 0, 16) && ReadBuilder(Image + (*Entry >> 32), Builder))
                {
                    Builder.Site = Image + (uint32_t)*Site;
                    Sample(Builder.Site);
                    Strings.Builders.push_back(std::move(Builder));
                }

                if (std::count_if(Entry, Last, IsCall) >= 1024)
                {
                    for (int32_t Tried = 0; Site != Last && Tried < 16; Site = std::find_if(Site + 1, Last, IsCall), Tried++)
                        Frequent.push_back(Image + (uint32_t)*Site);
                }

                Entry = Last;
            }

            for (int32_t Pass = 0; Pass < 2 && Strings.Keystreams.empty(); Pass++)
            {
                if (Pass)
                {
                    Counts.clear();
                    Found = 0;

                    for (auto Site : Frequent)
                        Sample(Site);
                }

                std::stable_sort(Counts.begin(), Counts.end(), [](const auto& A, const auto& B) { return A.second > B.second; });

                for (auto& [Keystream, Count] : Counts)
                {
                    if (Count >= 2)
                        Strings.Keystreams.push_back(Keystream);
                }
            }

            if (Strings.Keystreams.empty())
                return FEncryptedStrings();

            for (auto& Section : Sections())
            {
                if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                    continue;

                auto Begin = Image + Section.VirtualAddress;
                auto End = Begin + Section.Misc.VirtualSize;

                for (auto Address = Begin; Address + 11 <= End; Address++)
                {
                    auto Mod = Address[1] >> 6;
                    auto Rm = Address[1] & 7;

                    if (Address[0] != 0xC7 || (Address[1] & 0x38) || Mod == 3 || (Mod == 0 && Rm == 5) || (Mod == 0 && Rm == 4 && (Address[2] & 7) == 5))
                        continue;

                    auto Immediate = Address + 2 + (Rm == 4) + (Mod == 1 ? 1 : Mod == 2 ? 4 : 0);

                    Strings.Stores.push_back((uint64_t)*(const uint32_t*)Immediate << 32 | (uint32_t)(Address - Image));
                }
            }

            std::sort(Strings.Stores.begin(), Strings.Stores.end());

            auto& Loads = References()[1];

            for (auto& Section : Sections())
            {
                if (Section.Characteristics & IMAGE_SCN_MEM_EXECUTE)
                    continue;

                auto Entry = std::lower_bound(Loads.begin(), Loads.end(), (uint64_t)Section.VirtualAddress << 32);

                for (; Entry != Loads.end() && (*Entry >> 32) < (uint64_t)Section.VirtualAddress + Section.Misc.VirtualSize; Entry++)
                {
                    if (Strings.Blobs.empty() || Strings.Blobs.back() != (uint32_t)(*Entry >> 32))
                        Strings.Blobs.push_back((uint32_t)(*Entry >> 32));
                }
            }

            return Strings;
        }();

        return Strings;
    }

    static bool IsWritable(const void* Address)
    {
        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;

            if ((Section.Characteristics & IMAGE_SCN_MEM_WRITE) && !(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE) && Address >= Begin && Address < Begin + Section.Misc.VirtualSize)
                return true;
        }

        return false;
    }

    static const std::vector<uint64_t>& WritablePointers()
    {
        static auto Table = []
        {
            std::vector<uint64_t> Table;
            auto Image = Base();
            auto Size = ImageSize();

            for (auto& Section : Sections())
            {
                if (!(Section.Characteristics & IMAGE_SCN_MEM_WRITE) || (Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                    continue;

                for (auto Slot = (uint64_t)Section.VirtualAddress; Slot + sizeof(uint64_t) <= (uint64_t)Section.VirtualAddress + Section.Misc.VirtualSize; Slot += sizeof(uint64_t))
                {
                    auto Value = *(uint64_t*)(Image + Slot) - (uint64_t)Image;

                    if (Value < Size)
                        Table.push_back(Value << 32 | Slot);
                }
            }

            std::sort(Table.begin(), Table.end());

            return Table;
        }();

        return Table;
    }

    static const uint8_t* FindEncryptedRecord(const uint8_t* Site, size_t Size)
    {
        uint64_t Values[ZYDIS_REGISTER_MAX_VALUE + 1] = {};
        bool Known[ZYDIS_REGISTER_MAX_VALUE + 1] = {};
        std::vector<std::pair<const uint8_t*, const uint8_t*>> Copies;
        FInstruction Instruction;
        auto Address = Site;

        for (int32_t Count = 0; Count < 512 && Decode(Address, Instruction, false); Count++, Address += Instruction.Info.length)
        {
            auto Category = Instruction.Info.meta.category;
            auto Mnemonic = Instruction.Info.mnemonic;
            auto& Destination = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];
            ZyanU64 Target = 0;

            if (Count && (Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET || Category == ZYDIS_CATEGORY_UNCOND_BR))
                break;

            if (Mnemonic == ZYDIS_MNEMONIC_LEA && Destination.type == ZYDIS_OPERAND_TYPE_REGISTER && IsGeneralRegister(Destination.reg.value))
            {
                auto Register = LargestRegister(Destination.reg.value);
                auto bRelative = Source.mem.base == ZYDIS_REGISTER_RIP && ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Source, (ZyanU64)Address, &Target));
                auto bOffset = Source.mem.base != ZYDIS_REGISTER_NONE && Source.mem.base != ZYDIS_REGISTER_RIP && Known[LargestRegister(Source.mem.base)];

                Values[Register] = bRelative ? Target : bOffset ? Values[LargestRegister(Source.mem.base)] + Source.mem.disp.value : 0;
                Known[Register] = Source.mem.index == ZYDIS_REGISTER_NONE && (bRelative || bOffset);
                continue;
            }

            auto bCopy = Mnemonic == ZYDIS_MNEMONIC_MOV || Mnemonic == ZYDIS_MNEMONIC_MOVUPS || Mnemonic == ZYDIS_MNEMONIC_MOVAPS || Mnemonic == ZYDIS_MNEMONIC_MOVDQU || Mnemonic == ZYDIS_MNEMONIC_MOVDQA || Mnemonic == ZYDIS_MNEMONIC_MOVSD || Mnemonic == ZYDIS_MNEMONIC_MOVQ || Mnemonic == ZYDIS_MNEMONIC_MOVD || Mnemonic == ZYDIS_MNEMONIC_VMOVUPS || Mnemonic == ZYDIS_MNEMONIC_VMOVAPS || Mnemonic == ZYDIS_MNEMONIC_VMOVDQU || Mnemonic == ZYDIS_MNEMONIC_VMOVDQA;

            if (bCopy && Destination.type == ZYDIS_OPERAND_TYPE_MEMORY && Destination.mem.index == ZYDIS_REGISTER_NONE)
            {
                auto bRelative = Destination.mem.base == ZYDIS_REGISTER_RIP && ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Destination, (ZyanU64)Address, &Target));
                auto bOffset = Destination.mem.base != ZYDIS_REGISTER_NONE && Destination.mem.base != ZYDIS_REGISTER_RIP && Known[LargestRegister(Destination.mem.base)];
                auto Copy = (const uint8_t*)(bRelative ? Target : bOffset ? Values[LargestRegister(Destination.mem.base)] + Destination.mem.disp.value : 0);

                if ((bRelative || bOffset) && IsWritable(Copy))
                    Copies.emplace_back(Copy, Copy + Destination.size / 8);
            }

            bool Written[ZYDIS_REGISTER_MAX_VALUE + 1] = {};

            if (!WritesGeneralRegister(Instruction, Written))
                continue;

            for (size_t Register = 0; Register <= ZYDIS_REGISTER_MAX_VALUE; Register++)
                Known[Register] = Known[Register] && !Written[Register];
        }

        if (Copies.empty())
            return nullptr;

        std::sort(Copies.begin(), Copies.end());

        auto Covered = Copies.front().first;

        for (auto& [Begin, End] : Copies)
        {
            if (Begin <= Covered && End > Covered)
                Covered = End;
        }

        if ((size_t)(Covered - Copies.front().first) < Size)
            return nullptr;

        auto& Slots = WritablePointers();
        auto Key = (uint64_t)(Copies.front().first - Base()) << 32;

        for (auto Entry = std::lower_bound(Slots.begin(), Slots.end(), Key); Entry != Slots.end() && (*Entry >> 32) == (Key >> 32); Entry++)
        {
            if (auto Reference = FindReference(Base() + (uint32_t)*Entry))
                return (Reference[-1] & 0xFB) == 0x48 ? Reference - 1 : Reference;
        }

        return nullptr;
    }

    static std::vector<const uint8_t*> FindEncryptedStrings(std::span<const uint8_t> Plain, bool bPrefix)
    {
        auto& Strings = EncryptedStrings();
        std::vector<const uint8_t*> Sites;
        std::vector<int16_t> Cipher;

        if (Plain.size() < 4)
            return Sites;

        for (auto& Keystream : Strings.Keystreams)
        {
            for (auto& Builder : Strings.Builders)
            {
                if ((bPrefix ? Builder.Cipher.size() >= Plain.size() : Builder.Cipher.size() == Plain.size()) && Decrypts(Keystream, Builder.Cipher.data(), Plain))
                    Sites.push_back(Builder.Site);
            }

            for (int32_t First = 0; First < 256; First++)
            {
                uint64_t Encrypted = 0;
                auto Key = (uint8_t)First;

                for (int32_t Index = 0; Index < 4; Index++, Key = Keystream[Key])
                    Encrypted |= (uint64_t)(uint8_t)(Plain[Index] ^ Key) << (Index * 8);

                for (auto Entry = std::lower_bound(Strings.Stores.begin(), Strings.Stores.end(), Encrypted << 32); Entry != Strings.Stores.end() && (*Entry >> 32) == Encrypted; Entry++)
                {
                    auto Store = Base() + (uint32_t)*Entry;
                    auto Builder = std::upper_bound(Strings.Builders.begin(), Strings.Builders.end(), Store, [](const uint8_t* Address, const FEncryptedBuilder& Other) { return Address < Other.Begin; });

                    if (Builder != Strings.Builders.begin() && Store < Builder[-1].End)
                        continue;

                    for (auto Start : { Store, Store - 1 })
                    {
                        if ((Start == Store || (Store[-1] & 0xF0) == 0x40) && ReadStores(Start, Plain.size(), Cipher) && Decrypts(Keystream, Cipher.data(), Plain))
                        {
                            Sites.push_back(Start);
                            break;
                        }
                    }
                }
            }
        }

        for (auto& Keystream : Sites.empty() ? Strings.Keystreams : std::vector<std::array<uint8_t, 256>>())
        {
            for (auto Blob : Strings.Blobs)
            {
                auto Reference = Blob + Plain.size() <= ImageSize() && Decrypts(Keystream, Base() + Blob, Plain) ? FindReference(Base() + Blob) : nullptr;

                if (Reference)
                    Sites.push_back((Reference[-1] & 0xFB) == 0x48 ? Reference - 1 : Reference);
            }
        }

        for (auto& Site : Sites)
        {
            if (auto Record = FindEncryptedRecord(Site, Plain.size()))
                Site = Record;
        }

        std::sort(Sites.begin(), Sites.end());
        Sites.erase(std::unique(Sites.begin(), Sites.end()), Sites.end());

        return Sites;
    }

    static std::vector<const uint8_t*> FindEncryptedStrings(const wchar_t* Text)
    {
        auto Length = wcslen(Text);
        auto bPrefix = Text[Length - 1] == L'*';

        return FindEncryptedStrings({ (const uint8_t*)Text, (bPrefix ? Length - 1 : Length + 1) * sizeof(wchar_t) }, bPrefix);
    }

    static std::vector<const uint8_t*> FindEncryptedStrings(const char* Text)
    {
        auto Length = strlen(Text);
        auto bPrefix = Text[Length - 1] == '*';

        return FindEncryptedStrings({ (const uint8_t*)Text, bPrefix ? Length - 1 : Length + 1 }, bPrefix);
    }

    FCode String(const wchar_t* Text)
    {
        auto Reference = FindStringReference(Text);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        auto Sites = Reference ? std::vector<const uint8_t*>() : FindEncryptedStrings(Text);

        return FCode(FindLogCaller(Sites.empty() ? Reference : Sites.front()));
    }

    FCode String(const char* Text)
    {
        auto Reference = FindStringReference(Text);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        auto Sites = Reference ? std::vector<const uint8_t*>() : FindEncryptedStrings(Text);

        return FCode(FindLogCaller(Sites.empty() ? Reference : Sites.front()));
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

        for (auto Site : Found ? std::vector<const uint8_t*>() : FindEncryptedStrings(Text))
        {
            auto Entry = FindFunction(Site);

            if (!Entry)
                continue;

            auto EntrySize = FindFunctionEnd(Entry) - (Base() + Entry->BeginAddress);

            if (!Found || EntrySize < FoundSize)
            {
                Found = Site;
                FoundSize = EntrySize;
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
