#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cstdarg>
#include <cstdlib>
#include <memory>
#include <span>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
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

    static const std::vector<std::pair<uint64_t, uint32_t>>& Texts()
    {
        static auto Table = []
        {
            std::vector<std::pair<uint64_t, uint32_t>> Table;
            auto Image = Base();

            for (auto Entry : References()[1])
            {
                auto Target = (uint32_t)(Entry >> 32);

                for (auto& Section : Sections())
                {
                    if ((Section.Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE)) || Target < Section.VirtualAddress || Target >= Section.VirtualAddress + Section.Misc.VirtualSize)
                        continue;

                    uint64_t Key = 0;

                    for (uint32_t Index = 0; Index < 8; Index++)
                        Key = Key << 8 | (Target + Index < Section.VirtualAddress + Section.Misc.VirtualSize ? Image[Target + Index] : 0);

                    Table.emplace_back(Key, Target);
                }
            }

            std::sort(Table.begin(), Table.end());
            Table.erase(std::unique(Table.begin(), Table.end()), Table.end());

            return Table;
        }();

        return Table;
    }

    static uint64_t TextKey(const uint8_t* Address)
    {
        uint64_t Key = 0;

        for (uint32_t Index = 0; Index < 8; Index++)
            Key = Key << 8 | Address[Index];

        return Key;
    }

    static const std::vector<std::pair<uint64_t, uint32_t>>& TextKeys(int32_t Kind)
    {
        static auto Build = [](int32_t Kind)
        {
            std::vector<std::pair<uint64_t, uint32_t>> Table;
            auto Image = Base();
            auto Size = ImageSize();
            auto& Entries = References()[Kind];

            for (auto Entry = Entries.begin(); Entry != Entries.end(); Entry++)
            {
                auto Target = *Entry >> 32;

                if ((Entry == Entries.begin() || (Entry[-1] >> 32) != Target) && Target + 8 <= Size)
                    Table.emplace_back(TextKey(Image + Target), (uint32_t)Target);
            }

            std::sort(Table.begin(), Table.end());

            return Table;
        };

        static auto Leas = Build(1);
        static auto Loads = Build(3);

        return Kind == 1 ? Leas : Loads;
    }

    static const std::vector<std::pair<uint64_t, uint32_t>>& PointerTextKeys()
    {
        static auto Table = []
        {
            std::vector<std::pair<uint64_t, uint32_t>> Table;
            auto Image = Base();
            auto Size = ImageSize();
            auto& Entries = Pointers();

            for (size_t Index = 0; Index < Entries.size(); Index++)
            {
                if ((Entries[Index] >> 32) + 8 <= Size)
                    Table.emplace_back(TextKey(Image + (Entries[Index] >> 32)), (uint32_t)Index);
            }

            std::sort(Table.begin(), Table.end());

            return Table;
        }();

        return Table;
    }

    static const std::array<uint64_t, 256>& CodeByteCounts()
    {
        static auto Counts = []
        {
            std::array<uint64_t, 256> Counts{};

            for (auto& Section : Sections())
            {
                if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                    continue;

                auto Begin = Base() + Section.VirtualAddress;
                auto End = Begin + Section.Misc.VirtualSize;

                for (auto Address = Begin; Address < End; Address++)
                    Counts[*Address]++;
            }

            return Counts;
        }();

        return Counts;
    }

    static const std::array<uint64_t, 256>& ImageByteCounts()
    {
        static auto Counts = []
        {
            std::array<uint64_t, 256> Counts{};

            for (auto& Section : Sections())
            {
                auto Begin = Base() + Section.VirtualAddress;
                auto End = Begin + Section.Misc.VirtualSize;

                for (auto Address = Begin; Address < End; Address++)
                    Counts[*Address]++;
            }

            return Counts;
        }();

        return Counts;
    }

    static std::vector<const uint8_t*> FindWideTexts(const wchar_t* Text, size_t Size)
    {
        std::vector<const uint8_t*> Found;
        auto Bytes = (const uint8_t*)Text;
        auto& Counts = ImageByteCounts();
        size_t Rarest = 0;

        for (size_t Index = 1; Index < Size; Index++)
        {
            if (Counts[Bytes[Index]] < Counts[Bytes[Rarest]])
                Rarest = Index;
        }

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            if (Begin + Size > End)
                continue;

            auto Last = End - Size + Rarest;

            for (auto Hit = (const uint8_t*)memchr(Begin + Rarest, Bytes[Rarest], Last - (Begin + Rarest) + 1); Hit; Hit = Hit + 1 <= Last ? (const uint8_t*)memchr(Hit + 1, Bytes[Rarest], Last - Hit) : nullptr)
            {
                auto Address = Hit - Rarest;

                if (!((Address - Begin) % sizeof(wchar_t)) && !memcmp(Address, Text, Size))
                    Found.push_back(Address);
            }
        }

        return Found;
    }

    void Warm()
    {
        References();
        Pointers();
        Texts();
        TextKeys(1);
        PointerTextKeys();
        CodeByteCounts();
        ImageByteCounts();
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

        if (Size >= 8)
        {
            auto& Keys = TextKeys(1);
            auto Key = TextKey((const uint8_t*)Text);

            for (auto Entry = std::lower_bound(Keys.begin(), Keys.end(), std::make_pair(Key, 0u)); Entry != Keys.end() && Entry->first == Key && !Found; Entry++)
            {
                if (IsText(Image + Entry->second) && FindReference(Image + Entry->second))
                    Found = Image + Entry->second;
            }
        }
        else
        {
            for (auto Entry = Table.begin(); Entry != Table.end() && !Found; Entry++)
            {
                if ((Entry == Table.begin() || (Entry[-1] >> 32) != (*Entry >> 32)) && IsText(Image + (*Entry >> 32)) && FindReference(Image + (*Entry >> 32)))
                    Found = Image + (*Entry >> 32);
            }
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

        if (Size >= 8)
        {
            auto& Keys = TextKeys(3);
            auto Key = TextKey((const uint8_t*)Text);

            for (auto Entry = std::lower_bound(Keys.begin(), Keys.end(), std::make_pair(Key, 0u)); Entry != Keys.end() && Entry->first == Key && !Found; Entry++)
            {
                if (IsText(Image + Entry->second))
                    Found = Image + Entry->second;
            }
        }
        else
        {
            for (auto Entry = LoadTable.begin(); Entry != LoadTable.end() && !Found; Entry++)
            {
                if ((Entry == LoadTable.begin() || (Entry[-1] >> 32) != (*Entry >> 32)) && IsText(Image + (*Entry >> 32)))
                    Found = Image + (*Entry >> 32);
            }
        }

        if (Found)
            return FindIndexed(3, Found, [Found](const uint8_t* Address) { return InCode(Address, 0, 7) && Address[0] == 0x0F && (Address[1] == 0x10 || Address[1] == 0x28) && (Address[2] & 0xC7) == 0x05 && Address + 7 + *(int32_t*)(Address + 3) == Found; });

        if (Size >= 8)
        {
            auto& Keys = PointerTextKeys();
            auto Key = TextKey((const uint8_t*)Text);
            auto& Entries = Pointers();

            for (auto Entry = std::lower_bound(Keys.begin(), Keys.end(), std::make_pair(Key, 0u)); Entry != Keys.end() && Entry->first == Key; Entry++)
            {
                if (IsText(Image + (Entries[Entry->second] >> 32)))
                    return FindReference(Image + (uint32_t)Entries[Entry->second]);
            }

            return nullptr;
        }

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

        auto& Counts = CodeByteCounts();
        size_t Rarest = Bytes.size();

        for (size_t Index = 0; Index < Bytes.size(); Index++)
        {
            if (Bytes[Index] != -1 && (Rarest == Bytes.size() || Counts[Bytes[Index]] < Counts[Bytes[Rarest]]))
                Rarest = Index;
        }

        for (auto& Section : Sections())
        {
            if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;

            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            if (Rarest == Bytes.size())
            {
                if (Begin + Bytes.size() <= End)
                    return Begin;

                continue;
            }

            if (Begin + Bytes.size() > End)
                continue;

            auto Last = End - Bytes.size() + Rarest;

            for (auto Hit = (const uint8_t*)memchr(Begin + Rarest, Bytes[Rarest], Last - (Begin + Rarest) + 1); Hit; Hit = Hit + 1 <= Last ? (const uint8_t*)memchr(Hit + 1, Bytes[Rarest], Last - Hit) : nullptr)
            {
                auto Address = Hit - Rarest;
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

    static bool IsStackAdjustSwapped()
    {
        static auto bSwapped = FindPattern("48 83 C4 D8") != nullptr;

        return bSwapped;
    }

    static bool MatchSwapped(const FInstruction& Instruction, std::string_view Pattern)
    {
        if ((Instruction.Info.mnemonic != ZYDIS_MNEMONIC_ADD && Instruction.Info.mnemonic != ZYDIS_MNEMONIC_SUB) || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER || Instruction.Operands[0].reg.value != ZYDIS_REGISTER_RSP || Instruction.Operands[1].type != ZYDIS_OPERAND_TYPE_IMMEDIATE || Instruction.Operands[1].imm.value.s >= 0 || !IsStackAdjustSwapped())
            return false;

        char Text[64];
        snprintf(Text, sizeof(Text), "%s rsp, 0x%llx", Instruction.Info.mnemonic == ZYDIS_MNEMONIC_ADD ? "sub" : "add", (unsigned long long)-Instruction.Operands[1].imm.value.s);

        return Match(Text, Pattern);
    }

    static bool IsFrameFinishClang()
    {
        static auto bClang = FindPattern("48 83 F8 01 48 83 D8 FF") != nullptr;

        return bClang;
    }

    static bool IsFrameFinishStep(const uint8_t* Address, ZydisMnemonic Mnemonic, ZydisRegister Register, int64_t Value)
    {
        FInstruction Instruction;

        return Decode(Address, Instruction) && Instruction.Info.length == 4 && Instruction.Info.mnemonic == Mnemonic && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER && Instruction.Operands[0].reg.value == Register && Instruction.Operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Instruction.Operands[1].imm.value.s == Value;
    }

    static bool MatchFrameFinish(const uint8_t* Address, const FInstruction& Instruction, std::string_view Pattern)
    {
        static const char* const LowBytes[] = { "al", "cl", "dl", "bl", "spl", "bpl", "sil", "dil", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b" };
        char Text[sizeof(Instruction.Text) + 8];

        if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_SBB && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetClass(Instruction.Operands[0].reg.value) == ZYDIS_REGCLASS_GPR64 && IsFrameFinishStep(Address, ZYDIS_MNEMONIC_SBB, Instruction.Operands[0].reg.value, -1) && IsFrameFinishStep(Address - 4, ZYDIS_MNEMONIC_CMP, Instruction.Operands[0].reg.value, 1) && IsFrameFinishClang())
        {
            snprintf(Text, sizeof(Text), "setnz %s", LowBytes[ZydisRegisterGetId(Instruction.Operands[0].reg.value)]);

            return Match(Text, Pattern);
        }

        if (Instruction.Info.mnemonic == ZYDIS_MNEMONIC_MOV && Instruction.Operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY && Instruction.Operands[0].mem.index == ZYDIS_REGISTER_NONE && Instruction.Operands[0].mem.disp.value == 0x20 && Instruction.Operands[1].type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetClass(Instruction.Operands[1].reg.value) == ZYDIS_REGCLASS_GPR64 && IsFrameFinishStep(Address - 4, ZYDIS_MNEMONIC_SBB, Instruction.Operands[1].reg.value, -1) && IsFrameFinishStep(Address - 8, ZYDIS_MNEMONIC_CMP, Instruction.Operands[1].reg.value, 1) && IsFrameFinishClang())
        {
            snprintf(Text, sizeof(Text), "add%s", Instruction.Text + 3);

            return Match(Text, Pattern);
        }

        return false;
    }

    static bool Matches(const uint8_t* Address, std::string_view Instructions)
    {
        FInstruction Instruction;

        for (;;)
        {
            auto Separator = Instructions.find("; ");
            auto Pattern = Instructions.substr(0, Separator);

            if (!Decode(Address, Instruction) || (!Match(Instruction.Text, Pattern) && !MatchSwapped(Instruction, Pattern) && !MatchFrameFinish(Address, Instruction, Pattern)))
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

    static bool FindConstant(const uint8_t* Begin, const uint8_t* End, ZydisRegister Register, uint64_t& Value)
    {
        auto bKnown = false;
        FInstruction Instruction;

        for (auto Address = Begin; Address && Address < End; Address += Instruction.Info.length)
        {
            if (!Decode(Address, Instruction))
            {
                Instruction.Info.length = 1;
                continue;
            }

            for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count; Index++)
            {
                auto& Operand = Instruction.Operands[Index];

                if (Operand.type != ZYDIS_OPERAND_TYPE_REGISTER || !(Operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.reg.value) != Register)
                    continue;

                bKnown = Index == 0 && Instruction.Info.mnemonic == ZYDIS_MNEMONIC_MOV && Instruction.Operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE;
                Value = bKnown ? (Operand.size == 32 ? (uint32_t)Instruction.Operands[1].imm.value.u : Instruction.Operands[1].imm.value.u) : 0;
            }
        }

        return bKnown;
    }

    static const uint8_t* FindHalfSwap(const uint8_t* Begin, const uint8_t* Copy, ZydisRegister& Register, uint64_t& Key, std::vector<const uint8_t*>* Path)
    {
        FInstruction Instruction;

        if (!Decode(Copy, Instruction) || Instruction.Info.mnemonic != ZYDIS_MNEMONIC_MOV || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER || Instruction.Operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER || Instruction.Operands[0].size != 32 || Instruction.Operands[1].size != 32 || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Instruction.Operands[1].reg.value) != Register)
            return nullptr;

        auto Swapped = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Instruction.Operands[0].reg.value);
        auto bShiftLeft = false;
        auto bShiftRight = false;
        auto bHighKey = false;
        auto bLowKey = false;
        uint64_t HighKey = 0;
        uint64_t LowKey = 0;
        std::vector<const uint8_t*> Scanned = { Copy };

        if (Swapped == Register)
            return nullptr;

        for (auto Next = Copy + Instruction.Info.length; Scanned.size() < 12 && Decode(Next, Instruction); Next += Instruction.Info.length)
        {
            auto& Target = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];
            auto Mnemonic = Instruction.Info.mnemonic;
            auto TargetRegister = Target.type == ZYDIS_OPERAND_TYPE_REGISTER ? ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Target.reg.value) : ZYDIS_REGISTER_NONE;
            auto SourceRegister = Source.type == ZYDIS_OPERAND_TYPE_REGISTER ? ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value) : ZYDIS_REGISTER_NONE;
            auto bTouched = false;

            for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count; Index++)
            {
                auto& Operand = Instruction.Operands[Index];
                auto Used = Operand.type == ZYDIS_OPERAND_TYPE_REGISTER ? ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.reg.value) : ZYDIS_REGISTER_NONE;

                if (Used == Register || Used == Swapped || (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.base) == Register || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.base) == Swapped || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.index) == Register || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.index) == Swapped)))
                    bTouched = true;
            }

            if (Instruction.Info.meta.category == ZYDIS_CATEGORY_COND_BR || Instruction.Info.meta.category == ZYDIS_CATEGORY_UNCOND_BR || Instruction.Info.meta.category == ZYDIS_CATEGORY_CALL || Instruction.Info.meta.category == ZYDIS_CATEGORY_RET)
                return nullptr;

            Scanned.push_back(Next);

            if (!bTouched)
                continue;

            if (Target.size != 64 || Instruction.Info.operand_count_visible != 2)
                return nullptr;

            auto bKey = Mnemonic == ZYDIS_MNEMONIC_XOR && (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE || (SourceRegister != Register && SourceRegister != Swapped));

            if (Mnemonic == ZYDIS_MNEMONIC_SHL && TargetRegister == Swapped && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Source.imm.value.u == 32 && !bShiftLeft)
            {
                bShiftLeft = true;
            }
            else if (Mnemonic == ZYDIS_MNEMONIC_SHR && TargetRegister == Register && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Source.imm.value.u == 32 && !bShiftRight)
            {
                bShiftRight = true;
            }
            else if (bKey && TargetRegister == Swapped && bShiftLeft && !bHighKey)
            {
                if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                    HighKey = Source.imm.value.u;
                else if (!FindConstant(Begin ? Begin : Copy, Next, SourceRegister, HighKey))
                    return nullptr;

                bHighKey = true;
            }
            else if (bKey && TargetRegister == Register && bShiftRight && !bLowKey)
            {
                if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                    LowKey = Source.imm.value.u;
                else if (!FindConstant(Begin ? Begin : Copy, Next, SourceRegister, LowKey))
                    return nullptr;

                bLowKey = true;
            }
            else if (Mnemonic == ZYDIS_MNEMONIC_OR && ((TargetRegister == Swapped && SourceRegister == Register) || (TargetRegister == Register && SourceRegister == Swapped)) && bShiftLeft && bShiftRight && bHighKey && bLowKey && !(HighKey & 0xFFFFFFFFull) && !(LowKey >> 32))
            {
                if (Path)
                    Path->insert(Path->end(), Scanned.begin(), Scanned.end());

                Register = TargetRegister;
                Key = HighKey | LowKey;

                return Next;
            }
            else
                return nullptr;
        }

        return nullptr;
    }

    static bool FindPathConstant(const std::vector<const uint8_t*>& Path, ZydisRegister Register, uint64_t& Value)
    {
        auto bNarrow = false;
        FInstruction Instruction;

        for (auto Index = Path.size(); Index-- > 0;)
        {
            if (!Decode(Path[Index], Instruction))
                return false;

            auto bWrites = false;

            for (ZyanU8 Operand = 0; Operand < Instruction.Info.operand_count; Operand++)
            {
                auto& Written = Instruction.Operands[Operand];

                if (Written.type == ZYDIS_OPERAND_TYPE_REGISTER && (Written.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE) && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Written.reg.value) == Register)
                    bWrites = true;
            }

            if (!bWrites)
                continue;

            auto& Target = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];

            if (Instruction.Info.mnemonic != ZYDIS_MNEMONIC_MOV || Target.type != ZYDIS_OPERAND_TYPE_REGISTER || (Target.size != 32 && Target.size != 64))
                return false;

            bNarrow |= Target.size == 32;

            if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                Value = bNarrow ? (uint32_t)Source.imm.value.u : Source.imm.value.u;
                return true;
            }

            if (Source.type != ZYDIS_OPERAND_TYPE_REGISTER || ZydisRegisterGetClass(Source.reg.value) == ZYDIS_REGCLASS_GPR16 || ZydisRegisterGetClass(Source.reg.value) == ZYDIS_REGCLASS_GPR8)
                return false;

            Register = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value);
        }

        return false;
    }

    FDecoder FCode::DecoderKeepingZero(const uint8_t* Test) const
    {
        FDecoder Result;
        FDecoder Empty;
        FInstruction Instruction;
        std::vector<const uint8_t*> Path;

        if (!Decode(Address, Instruction) || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
            return Empty;

        auto Register = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Instruction.Operands[0].reg.value);
        auto Begin = FindRange(Address).first;

        for (auto Next = Begin ? Begin : Address; Next < Test && Decode(Next, Instruction); Next += Instruction.Info.length)
            Path.push_back(Next);

        if (!Decode(Test, Instruction) || Instruction.Info.operand_count_visible != 2 || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER || Instruction.Operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER || Instruction.Operands[0].reg.value != Instruction.Operands[1].reg.value || (Instruction.Operands[0].size != 32 && Instruction.Operands[0].size != 64))
            return Empty;

        auto Width = (uint8_t)Instruction.Operands[0].size;
        auto Branch = Test + Instruction.Info.length;

        if (!Decode(Branch, Instruction) || (Instruction.Info.mnemonic != ZYDIS_MNEMONIC_JZ && Instruction.Info.mnemonic != ZYDIS_MNEMONIC_JNZ) || Instruction.Operands[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
            return Empty;

        ZyanU64 Taken = 0;

        if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Instruction.Operands[0], (ZyanU64)Branch, &Taken)))
            return Empty;

        Path.push_back(Test);
        Path.push_back(Branch);
        Result.Steps[Result.Count++] = { FDecoder::EOperation::Test, Width, 0 };

        auto bSetsZero = false;
        auto bSelectNext = false;

        for (auto Next = Instruction.Info.mnemonic == ZYDIS_MNEMONIC_JNZ ? (const uint8_t*)Taken : Branch + Instruction.Info.length; Result.Count < (int32_t)std::size(Result.Steps) && Decode(Next, Instruction); Next += Instruction.Info.length)
        {
            auto Category = Instruction.Info.meta.category;
            auto Mnemonic = Instruction.Info.mnemonic;
            auto& Target = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];
            auto bTouched = false;

            for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count; Index++)
            {
                auto& Operand = Instruction.Operands[Index];

                if ((Operand.type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.reg.value) == Register) || (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.base) == Register || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.index) == Register)))
                    bTouched = true;
            }

            if (Mnemonic == ZYDIS_MNEMONIC_JNZ && bSetsZero && Target.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                ZyanU64 Skip = 0;
                FInstruction Move;
                auto After = Next + Instruction.Info.length;

                if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(&Instruction.Info, &Target, (ZyanU64)Next, &Skip)) || !Decode(After, Move) || Move.Info.mnemonic != ZYDIS_MNEMONIC_MOV || Move.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Move.Operands[0].reg.value) != Register || Skip != (ZyanU64)(After + Move.Info.length))
                    return Empty;

                uint64_t Value = 0;
                Path.push_back(Next);

                if (Move.Operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                    Value = Move.Operands[1].imm.value.u;
                else if (Move.Operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER || !FindPathConstant(Path, ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Move.Operands[1].reg.value), Value))
                    return Empty;

                Result.Steps[Result.Count++] = { FDecoder::EOperation::Select, Width, Width == 32 ? (uint32_t)Value : Value };

                return Result;
            }

            if (Category == ZYDIS_CATEGORY_COND_BR || Category == ZYDIS_CATEGORY_UNCOND_BR || Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET)
                return Empty;

            if (!bTouched)
            {
                Path.push_back(Next);
                continue;
            }

            if (Mnemonic == ZYDIS_MNEMONIC_CMOVNZ && bSetsZero && Target.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value) == Register)
            {
                uint64_t Value = 0;

                if (!FindPathConstant(Path, ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Target.reg.value), Value))
                    return Empty;

                Result.Steps[Result.Count++] = { FDecoder::EOperation::Select, Width, Width == 32 ? (uint32_t)Value : Value };

                return Result;
            }

            if (bSelectNext)
                return Empty;

            auto bSelf = Target.type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Target.reg.value) == Register && Target.size == Width;
            auto bDecrement = (bSelf && Mnemonic == ZYDIS_MNEMONIC_DEC) || (bSelf && Mnemonic == ZYDIS_MNEMONIC_SUB && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Source.imm.value.s == 1) || (bSelf && Mnemonic == ZYDIS_MNEMONIC_ADD && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && Source.imm.value.s == -1) || (bSelf && Mnemonic == ZYDIS_MNEMONIC_LEA && Source.mem.index == ZYDIS_REGISTER_NONE && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.mem.base) == Register && Source.mem.disp.value == -1);

            if (Result.Count == 1)
            {
                if (!bDecrement && bSelf && Instruction.Info.operand_count_visible == 2 && (Mnemonic == ZYDIS_MNEMONIC_ADD || Mnemonic == ZYDIS_MNEMONIC_SUB))
                {
                    uint64_t Value = 0;

                    if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                        Value = Source.imm.value.u;
                    else if (Source.type != ZYDIS_OPERAND_TYPE_REGISTER || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value) == Register || !FindPathConstant(Path, ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value), Value))
                        return Empty;

                    Result.Steps[Result.Count++] = { Mnemonic == ZYDIS_MNEMONIC_ADD ? FDecoder::EOperation::Add : FDecoder::EOperation::Sub, Width, Width == 32 ? (uint32_t)Value : Value };
                    bSetsZero = true;
                    bSelectNext = true;
                    Path.push_back(Next);
                    continue;
                }

                if (!bDecrement)
                    return Empty;

                Result.Steps[Result.Count++] = { FDecoder::EOperation::Sub, Width, 1 };
                bSetsZero = Mnemonic != ZYDIS_MNEMONIC_LEA;
                Path.push_back(Next);
                continue;
            }

            uint64_t SwapKey = 0;

            if (auto SwapEnd = Result.Count + 2 <= (int32_t)std::size(Result.Steps) ? FindHalfSwap(Begin, Next, Register, SwapKey, &Path) : nullptr)
            {
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Ror, 64, 32 };
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Xor, 64, SwapKey };
                bSetsZero = true;
                Next = SwapEnd;
                Decode(Next, Instruction);
                continue;
            }

            if (!bSelf)
                return Empty;

            if (Instruction.Info.operand_count_visible == 1 && (Mnemonic == ZYDIS_MNEMONIC_NOT || Mnemonic == ZYDIS_MNEMONIC_NEG || Mnemonic == ZYDIS_MNEMONIC_BSWAP || Mnemonic == ZYDIS_MNEMONIC_DEC || Mnemonic == ZYDIS_MNEMONIC_INC))
            {
                auto Operation = Mnemonic == ZYDIS_MNEMONIC_NOT ? FDecoder::EOperation::Not : Mnemonic == ZYDIS_MNEMONIC_NEG ? FDecoder::EOperation::Neg : Mnemonic == ZYDIS_MNEMONIC_BSWAP ? FDecoder::EOperation::Bswap : Mnemonic == ZYDIS_MNEMONIC_DEC ? FDecoder::EOperation::Sub : FDecoder::EOperation::Add;
                Result.Steps[Result.Count++] = { Operation, Width, Mnemonic == ZYDIS_MNEMONIC_DEC || Mnemonic == ZYDIS_MNEMONIC_INC ? 1ull : 0ull };
                bSetsZero = Mnemonic != ZYDIS_MNEMONIC_NOT && Mnemonic != ZYDIS_MNEMONIC_BSWAP;
                Path.push_back(Next);
                continue;
            }

            if (Instruction.Info.operand_count_visible == 2 && (Mnemonic == ZYDIS_MNEMONIC_XOR || Mnemonic == ZYDIS_MNEMONIC_ADD || Mnemonic == ZYDIS_MNEMONIC_SUB || Mnemonic == ZYDIS_MNEMONIC_ROR || Mnemonic == ZYDIS_MNEMONIC_ROL))
            {
                uint64_t Value = 0;

                if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                    Value = Source.imm.value.u;
                else if (Source.type != ZYDIS_OPERAND_TYPE_REGISTER || Mnemonic == ZYDIS_MNEMONIC_ROR || Mnemonic == ZYDIS_MNEMONIC_ROL || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value) == Register || !FindPathConstant(Path, ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value), Value))
                    return Empty;

                auto Operation = Mnemonic == ZYDIS_MNEMONIC_XOR ? FDecoder::EOperation::Xor : Mnemonic == ZYDIS_MNEMONIC_ADD ? FDecoder::EOperation::Add : Mnemonic == ZYDIS_MNEMONIC_SUB ? FDecoder::EOperation::Sub : Mnemonic == ZYDIS_MNEMONIC_ROR ? FDecoder::EOperation::Ror : FDecoder::EOperation::Rol;
                Result.Steps[Result.Count++] = { Operation, Width, Width == 32 ? (uint32_t)Value : Value };
                bSetsZero = Mnemonic != ZYDIS_MNEMONIC_ROR && Mnemonic != ZYDIS_MNEMONIC_ROL;
                Path.push_back(Next);
                continue;
            }

            return Empty;
        }

        return Empty;
    }

    FDecoder FCode::Decoder() const
    {
        FDecoder Result;
        FInstruction Load;

        if (!Address || !Decode(Address, Load) || Load.Info.operand_count_visible != 2 || Load.Operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER || Load.Operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
            return Result;

        auto Register = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Load.Operands[0].reg.value);
        auto Begin = FindRange(Address).first;
        FInstruction Instruction;

        for (auto Next = Address + Load.Info.length; Result.Count < (int32_t)std::size(Result.Steps) && Decode(Next, Instruction); Next += Instruction.Info.length)
        {
            auto Category = Instruction.Info.meta.category;

            if (Category == ZYDIS_CATEGORY_COND_BR || Category == ZYDIS_CATEGORY_UNCOND_BR || Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET)
                break;

            const ZydisDecodedOperand* Memory = nullptr;
            auto bTouched = false;

            for (ZyanU8 Index = 0; Index < Instruction.Info.operand_count; Index++)
            {
                auto& Operand = Instruction.Operands[Index];

                if (Operand.type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.reg.value) == Register)
                    bTouched = true;

                if (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && (ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.base) == Register || ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Operand.mem.index) == Register))
                {
                    bTouched = true;
                    Memory = &Operand;
                }
            }

            if (!bTouched)
                continue;

            if (!Result.Count && Instruction.Info.mnemonic == ZYDIS_MNEMONIC_TEST)
                return DecoderKeepingZero(Next);

            uint64_t SwapKey = 0;

            if (auto SwapEnd = Result.Count + 2 <= (int32_t)std::size(Result.Steps) ? FindHalfSwap(Begin, Next, Register, SwapKey, nullptr) : nullptr)
            {
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Ror, 64, 32 };
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Xor, 64, SwapKey };
                Next = SwapEnd;
                Decode(Next, Instruction);
                continue;
            }

            auto& Target = Instruction.Operands[0];
            auto& Source = Instruction.Operands[1];
            auto Mnemonic = Instruction.Info.mnemonic;
            auto Width = (uint8_t)Target.size;
            auto bSelf = Target.type == ZYDIS_OPERAND_TYPE_REGISTER && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Target.reg.value) == Register && (Width == 32 || Width == 64);
            uint64_t Value = 0;

            if (bSelf && Instruction.Info.operand_count_visible == 1 && (Mnemonic == ZYDIS_MNEMONIC_NOT || Mnemonic == ZYDIS_MNEMONIC_NEG || Mnemonic == ZYDIS_MNEMONIC_BSWAP))
            {
                Result.Steps[Result.Count++] = { Mnemonic == ZYDIS_MNEMONIC_NOT ? FDecoder::EOperation::Not : Mnemonic == ZYDIS_MNEMONIC_NEG ? FDecoder::EOperation::Neg : FDecoder::EOperation::Bswap, Width, 0 };
                continue;
            }

            if (bSelf && Instruction.Info.operand_count_visible == 2 && (Mnemonic == ZYDIS_MNEMONIC_XOR || Mnemonic == ZYDIS_MNEMONIC_ADD || Mnemonic == ZYDIS_MNEMONIC_SUB || Mnemonic == ZYDIS_MNEMONIC_ROR || Mnemonic == ZYDIS_MNEMONIC_ROL))
            {
                auto bConstant = Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE;

                if (bConstant)
                    Value = Source.imm.value.u;
                else if (Source.type == ZYDIS_OPERAND_TYPE_REGISTER && Mnemonic != ZYDIS_MNEMONIC_ROR && Mnemonic != ZYDIS_MNEMONIC_ROL && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value) != Register)
                    bConstant = FindConstant(Begin ? Begin : Address, Next, ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Source.reg.value), Value);

                if (bConstant)
                {
                    auto Operation = Mnemonic == ZYDIS_MNEMONIC_XOR ? FDecoder::EOperation::Xor : Mnemonic == ZYDIS_MNEMONIC_ADD ? FDecoder::EOperation::Add : Mnemonic == ZYDIS_MNEMONIC_SUB ? FDecoder::EOperation::Sub : Mnemonic == ZYDIS_MNEMONIC_ROR ? FDecoder::EOperation::Ror : FDecoder::EOperation::Rol;
                    Result.Steps[Result.Count++] = { Operation, Width, Width == 32 ? (uint32_t)Value : Value };
                    continue;
                }
            }

            if (bSelf && Mnemonic == ZYDIS_MNEMONIC_LEA && Memory && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Memory->mem.base) == Register && Memory->mem.index == ZYDIS_REGISTER_NONE)
            {
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Add, Width, Width == 32 ? (uint32_t)Memory->mem.disp.value : (uint64_t)Memory->mem.disp.value };
                continue;
            }

            if (Result.Count && Memory && ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Memory->mem.base) == Register && Memory->mem.index != ZYDIS_REGISTER_NONE && Memory->mem.disp.value)
                Result.Steps[Result.Count++] = { FDecoder::EOperation::Add, 64, (uint64_t)Memory->mem.disp.value };

            break;
        }

        return Result;
    }

    uint64_t FDecoder::operator()(uint64_t Value) const
    {
        for (int32_t Index = 0; Index < Count; Index++)
        {
            auto& Step = Steps[Index];
            auto Mask = Step.Width == 64 ? ~0ull : 0xFFFFFFFFull;
            auto Low = Value & Mask;

            switch (Step.Operation)
            {
            case EOperation::Xor:
                Low ^= Step.Value;
                break;
            case EOperation::Add:
                Low += Step.Value;
                break;
            case EOperation::Sub:
                Low -= Step.Value;
                break;
            case EOperation::Ror:
                Low = Step.Width == 64 ? std::rotr(Low, (int)(Step.Value & 63)) : std::rotr((uint32_t)Low, (int)(Step.Value & 31));
                break;
            case EOperation::Rol:
                Low = Step.Width == 64 ? std::rotl(Low, (int)(Step.Value & 63)) : std::rotl((uint32_t)Low, (int)(Step.Value & 31));
                break;
            case EOperation::Not:
                Low = ~Low;
                break;
            case EOperation::Neg:
                Low = 0 - Low;
                break;
            case EOperation::Bswap:
                Low = Step.Width == 64 ? _byteswap_uint64(Low) : _byteswap_ulong((uint32_t)Low);
                break;
            case EOperation::Test:
                if (!Low)
                    return 0;
                break;
            case EOperation::Select:
                if (!Low)
                    Low = Step.Value;
                break;
            }

            Value = Low & Mask;
        }

        return Value;
    }

    int32_t FDecoder::Describe(wchar_t* Text, int32_t Size) const
    {
        static const wchar_t* const Names[] = { L"xor", L"add", L"sub", L"ror", L"rol", L"not", L"neg", L"bswap", L"test", L"select" };
        int32_t Length = 0;

        if (Size > 0)
            Text[0] = 0;

        for (int32_t Index = 0; Index < Count && Length >= 0 && Length < Size; Index++)
        {
            auto& Step = Steps[Index];
            auto Written = Step.Operation >= EOperation::Not && Step.Operation != EOperation::Select ? _snwprintf_s(Text + Length, Size - Length, _TRUNCATE, L"%s%s%d", Index ? L", " : L"", Names[(int32_t)Step.Operation], Step.Width) : _snwprintf_s(Text + Length, Size - Length, _TRUNCATE, L"%s%s%d 0x%llX", Index ? L", " : L"", Names[(int32_t)Step.Operation], Step.Width, (unsigned long long)Step.Value);

            Length = Written < 0 ? Size : Length + Written;
        }

        return Count;
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
            if (Address != Reference && (Instruction.Info.meta.category == ZYDIS_CATEGORY_COND_BR || Instruction.Info.meta.category == ZYDIS_CATEGORY_UNCOND_BR || Instruction.Info.mnemonic == ZYDIS_MNEMONIC_RET))
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

    static std::string NarrowText(const wchar_t* Text)
    {
        std::string Narrow;

        for (auto Char = Text; *Char; Char++)
        {
            if (*Char >= 0x80)
                return std::string();

            Narrow.push_back((char)*Char);
        }

        return Narrow;
    }

    static const uint8_t* FindText(const std::string& Narrow, const uint8_t* After)
    {
        auto Image = Base();
        auto& Table = Texts();
        auto Size = Narrow.back() == '*' ? Narrow.size() - 1 : Narrow.size() + 1;
        auto Known = std::min<size_t>(Size, 8);
        uint64_t Low = 0;
        const uint8_t* Found = nullptr;

        for (size_t Index = 0; Index < 8; Index++)
            Low = Low << 8 | (Index < Known ? (uint8_t)Narrow.c_str()[Index] : 0);

        auto High = Known < 8 ? Low | (~0ull >> (Known * 8)) : Low;

        for (auto Entry = std::lower_bound(Table.begin(), Table.end(), std::make_pair(Low, 0u)); Entry != Table.end() && Entry->first <= High; Entry++)
        {
            auto Address = Image + Entry->second;

            if (Address <= After || (Found && Address >= Found))
                continue;

            for (auto& Section : Sections())
            {
                auto Begin = Image + Section.VirtualAddress;

                if (Address >= Begin && Address + Size <= Begin + Section.Misc.VirtualSize && !memcmp(Address, Narrow.c_str(), Size))
                    Found = Address;
            }
        }

        return Found;
    }

    struct FOperation
    {
        ZydisDecodedInstruction Info;
        ZydisDecodedOperand Operands[ZYDIS_MAX_OPERAND_COUNT];
    };

    static const ZydisDecoder& OperationDecoder()
    {
        static auto Decoder = []
        {
            ZydisDecoder Decoder;
            ZydisDecoderInit(&Decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

            return Decoder;
        }();

        return Decoder;
    }

    static bool Decode(const uint8_t* Address, FOperation& Operation)
    {
        return ZYAN_SUCCESS(ZydisDecoderDecodeFull(&OperationDecoder(), Address, ZYDIS_MAX_INSTRUCTION_LENGTH, &Operation.Info, Operation.Operands));
    }

    static bool Decode(const uint8_t* Address, ZydisDecodedInstruction& Info)
    {
        ZydisDecoderContext Context;

        return ZYAN_SUCCESS(ZydisDecoderDecodeInstruction(&OperationDecoder(), &Context, Address, ZYDIS_MAX_INSTRUCTION_LENGTH, &Info));
    }

    static uint64_t Bits(uint32_t Size)
    {
        return Size >= 64 ? ~0ull : (1ull << Size) - 1;
    }

    class FMachine
    {
    public:
        FMachine()
        {
            Image = (uint64_t)Base();
            Size = ImageSize();

            for (auto& Section : Sections())
            {
                if (!(Section.Characteristics & IMAGE_SCN_MEM_WRITE))
                    Constants.emplace_back(Section.VirtualAddress, Section.VirtualAddress + Section.Misc.VirtualSize);
            }
        }

        void Reset()
        {
            memset(Known, 0, sizeof(Known));
            memset(VectorKnown, 0, sizeof(VectorKnown));
            memset(Aliases, 0, sizeof(Aliases));
            Registers[4] = 0x200000000000;
            Registers[5] = 0x300000000000;
            Known[4] = 0xFF;
            Known[5] = 0xFF;
            bFlags = false;
            Tested = ZYDIS_REGISTER_NONE;
            bRecord = false;
            Low = ~0ull;
            High = 0;
            Generation++;
        }

        uint64_t StackPointer() const
        {
            return Registers[4];
        }

        void SetStackPointer(uint64_t Value)
        {
            Registers[4] = Value;
            Known[4] = 0xFF;
        }

        void Clobber()
        {
            for (auto Index : { 0, 1, 2, 8, 9, 10, 11 })
            {
                Known[Index] = 0;
                Aliases[Index] = 0;
            }

            for (auto Index = 0; Index < 6; Index++)
                VectorKnown[Index] = 0;

            bFlags = false;
            Tested = ZYDIS_REGISTER_NONE;
        }

        struct FState
        {
            uint64_t Registers[16];
            uint8_t Known[16];
        };

        void Capture(FState& State) const
        {
            memcpy(State.Registers, Registers, sizeof(Registers));
            memcpy(State.Known, Known, sizeof(Known));
        }

        void Apply(const FState& State)
        {
            memcpy(Registers, State.Registers, sizeof(Registers));
            memcpy(Known, State.Known, sizeof(Known));
            memset(VectorKnown, 0, sizeof(VectorKnown));
            memset(Aliases, 0, sizeof(Aliases));
            bFlags = false;
            Tested = ZYDIS_REGISTER_NONE;
        }

        struct FContext
        {
            uint64_t Registers[16];
            uint8_t Known[16];
            uint8_t Vectors[16][16];
            uint16_t VectorKnown[16];
            uint32_t Aliases[16];
            ZydisRegister Tested;
            uint64_t TestedValue;
            bool bFlags;
            bool bZero;
            bool bSign;
            bool bCarry;
            bool bOverflow;
        };

        void Capture(FContext& Context) const
        {
            memcpy(Context.Registers, Registers, sizeof(Registers));
            memcpy(Context.Known, Known, sizeof(Known));
            memcpy(Context.Vectors, Vectors, sizeof(Vectors));
            memcpy(Context.VectorKnown, VectorKnown, sizeof(VectorKnown));
            memcpy(Context.Aliases, Aliases, sizeof(Aliases));
            Context.Tested = Tested;
            Context.TestedValue = TestedValue;
            Context.bFlags = bFlags;
            Context.bZero = bZero;
            Context.bSign = bSign;
            Context.bCarry = bCarry;
            Context.bOverflow = bOverflow;
        }

        void Apply(const FContext& Context)
        {
            memcpy(Registers, Context.Registers, sizeof(Registers));
            memcpy(Known, Context.Known, sizeof(Known));
            memcpy(Vectors, Context.Vectors, sizeof(Vectors));
            memcpy(VectorKnown, Context.VectorKnown, sizeof(VectorKnown));
            memcpy(Aliases, Context.Aliases, sizeof(Aliases));
            Tested = Context.Tested;
            TestedValue = Context.TestedValue;
            bFlags = Context.bFlags;
            bZero = Context.bZero;
            bSign = Context.bSign;
            bCarry = Context.bCarry;
            bOverflow = Context.bOverflow;
        }

        void Preserve(const FState& State)
        {
            for (auto Index : { 3, 4, 5, 6, 7, 12, 13, 14, 15 })
            {
                Registers[Index] = State.Registers[Index];
                Known[Index] = State.Known[Index];
                Aliases[Index] = 0;
            }
        }

        void Save()
        {
            Capture(Saved);
        }

        void Restore()
        {
            Apply(Saved);
        }

        void Refine(ZydisMnemonic Mnemonic, bool bTaken)
        {
            if (Tested == ZYDIS_REGISTER_NONE || !((Mnemonic == ZYDIS_MNEMONIC_JNZ && !bTaken) || (Mnemonic == ZYDIS_MNEMONIC_JZ && bTaken)))
                return;

            auto Slot = Describe(Tested).Slot;
            auto Group = Slot >= 0 ? Aliases[Slot] : 0;
            auto Value = TestedValue;

            Write(Tested, Value, true);

            for (auto Index = 0; Group && Index < 16; Index++)
            {
                if (Aliases[Index] == Group)
                {
                    Registers[Index] = Value;
                    Known[Index] = 0xFF;
                }
            }
        }

        void Record(bool bEnable)
        {
            bRecord = bEnable;

            if (bEnable)
            {
                Low = ~0ull;
                High = 0;
                Stores.clear();
            }
        }

        void Copy(bool bEnable)
        {
            bCopy = bEnable;

            if (bEnable)
            {
                CopyLow = ~0ull;
                Undo.clear();
            }
        }

        void Rollback()
        {
            for (auto Entry = Undo.rbegin(); Entry != Undo.rend(); ++Entry)
            {
                if (auto Found = Page(Entry->Address, false))
                {
                    Found->Stamps[Entry->Address & 0xFFF] = Entry->Stamp;
                    Found->Bytes[Entry->Address & 0xFFF] = Entry->Byte;
                }
            }

            Undo.clear();
        }

        uint32_t Copied(const std::string& Text)
        {
            std::string Bytes(Text.size(), 0);

            return CopyLow != ~0ull && Load(CopyLow, (uint32_t)Bytes.size(), (uint8_t*)Bytes.data()) && Bytes == Text ? (uint32_t)(CopyLow - Image) : 0;
        }

        uint64_t Written() const
        {
            return Low <= High ? High - Low + 1 : 0;
        }

        bool Overflowed() const
        {
            return Low <= High && High - Low >= 0x2000;
        }

        bool Recorded(std::string& Text)
        {
            if (Low > High || Overflowed())
                return false;

            std::sort(Stores.begin(), Stores.end());

            auto First = ~0ull;
            uint64_t Last = 0;

            for (size_t Index = 0; Index < Stores.size(); Index++)
            {
                if ((Index && Stores[Index - 1] == Stores[Index]) || (Index + 1 < Stores.size() && Stores[Index + 1] == Stores[Index]))
                    continue;

                First = std::min<uint64_t>(First, Stores[Index].first);
                Last = std::max<uint64_t>(Last, Stores[Index].first + Stores[Index].second - 1);
            }

            if (First > Last)
                return false;

            Text.resize(Last - First + 1);

            return Load(First, (uint32_t)Text.size(), (uint8_t*)Text.data());
        }

        bool Condition(ZydisMnemonic Mnemonic, bool& bTaken) const
        {
            if (!bFlags)
                return false;

            switch (Mnemonic)
            {
            case ZYDIS_MNEMONIC_JZ:
                bTaken = bZero;
                return true;
            case ZYDIS_MNEMONIC_JNZ:
                bTaken = !bZero;
                return true;
            case ZYDIS_MNEMONIC_JB:
                bTaken = bCarry;
                return true;
            case ZYDIS_MNEMONIC_JNB:
                bTaken = !bCarry;
                return true;
            case ZYDIS_MNEMONIC_JBE:
                bTaken = bCarry || bZero;
                return true;
            case ZYDIS_MNEMONIC_JNBE:
                bTaken = !bCarry && !bZero;
                return true;
            case ZYDIS_MNEMONIC_JL:
                bTaken = bSign != bOverflow;
                return true;
            case ZYDIS_MNEMONIC_JNL:
                bTaken = bSign == bOverflow;
                return true;
            case ZYDIS_MNEMONIC_JLE:
                bTaken = bZero || bSign != bOverflow;
                return true;
            case ZYDIS_MNEMONIC_JNLE:
                bTaken = !bZero && bSign == bOverflow;
                return true;
            case ZYDIS_MNEMONIC_JS:
                bTaken = bSign;
                return true;
            case ZYDIS_MNEMONIC_JNS:
                bTaken = !bSign;
                return true;
            case ZYDIS_MNEMONIC_JO:
                bTaken = bOverflow;
                return true;
            case ZYDIS_MNEMONIC_JNO:
                bTaken = !bOverflow;
                return true;
            default:
                return false;
            }
        }

        void Execute(const FOperation& Operation, const uint8_t* Address)
        {
            auto& Info = Operation.Info;
            auto& Target = Operation.Operands[0];
            auto& Source = Operation.Operands[1];
            auto Width = Target.size;
            auto Mask = Bits(Width);
            uint64_t Left = 0;
            uint64_t Right = 0;

            switch (Info.mnemonic)
            {
            case ZYDIS_MNEMONIC_NOP:
            case ZYDIS_MNEMONIC_INT3:
                return;
            case ZYDIS_MNEMONIC_MOV:
                if (Width <= 64)
                {
                    auto bKnown = Get(Operation, Source, Address, Right);
                    Set(Operation, Target, Address, Right, bKnown);

                    if (Width >= 32 && Target.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.type == ZYDIS_OPERAND_TYPE_REGISTER && Describe(Target.reg.value).Slot >= 0 && Describe(Source.reg.value).Slot >= 0)
                    {
                        auto& Group = Aliases[Describe(Source.reg.value).Slot];
                        Group = Group ? Group : ++Alias;
                        Aliases[Describe(Target.reg.value).Slot] = Group;
                    }

                    return;
                }
                break;
            case ZYDIS_MNEMONIC_MOVZX:
            {
                auto bKnown = Get(Operation, Source, Address, Right);
                Set(Operation, Target, Address, Right & Bits(Source.size), bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_MOVSX:
            case ZYDIS_MNEMONIC_MOVSXD:
            {
                auto bKnown = Get(Operation, Source, Address, Right) && Source.size && Source.size <= 64;
                auto Shift = 64 - (bKnown ? Source.size : 64);
                Set(Operation, Target, Address, (uint64_t)((int64_t)(Right << Shift) >> Shift), bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_LEA:
            {
                auto bKnown = Effective(Operation, Source, Address, Right);
                Set(Operation, Target, Address, Right, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_ADD:
            case ZYDIS_MNEMONIC_SUB:
            case ZYDIS_MNEMONIC_XOR:
            case ZYDIS_MNEMONIC_AND:
            case ZYDIS_MNEMONIC_OR:
            case ZYDIS_MNEMONIC_CMP:
            case ZYDIS_MNEMONIC_TEST:
            {
                if (!Width || Width > 64 || Info.operand_count_visible != 2)
                    break;

                auto bKnown = Get(Operation, Target, Address, Left);
                bKnown = Get(Operation, Source, Address, Right) && bKnown;

                if (Target.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.type == ZYDIS_OPERAND_TYPE_REGISTER && Target.reg.value == Source.reg.value && (Info.mnemonic == ZYDIS_MNEMONIC_XOR || Info.mnemonic == ZYDIS_MNEMONIC_SUB))
                {
                    bKnown = true;
                    Left = 0;
                    Right = 0;
                }

                if (Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE && ((Info.mnemonic == ZYDIS_MNEMONIC_OR && (Right & Mask) == Mask) || (Info.mnemonic == ZYDIS_MNEMONIC_AND && !(Right & Mask))))
                {
                    bKnown = true;
                    Left = Right;
                }

                Left &= Mask;
                Right &= Mask;

                auto Sign = 1ull << (Width - 1);
                uint64_t Result = 0;
                auto bCarryOut = false;
                auto bOverflowOut = false;

                switch (Info.mnemonic)
                {
                case ZYDIS_MNEMONIC_ADD:
                    Result = (Left + Right) & Mask;
                    bCarryOut = Result < Left;
                    bOverflowOut = ((Left ^ Result) & (Right ^ Result) & Sign) != 0;
                    break;
                case ZYDIS_MNEMONIC_SUB:
                case ZYDIS_MNEMONIC_CMP:
                    Result = (Left - Right) & Mask;
                    bCarryOut = Left < Right;
                    bOverflowOut = ((Left ^ Right) & (Left ^ Result) & Sign) != 0;
                    break;
                case ZYDIS_MNEMONIC_XOR:
                    Result = Left ^ Right;
                    break;
                case ZYDIS_MNEMONIC_OR:
                    Result = Left | Right;
                    break;
                default:
                    Result = Left & Right;
                    break;
                }

                SetFlags(Result, Width, bCarryOut, bOverflowOut, bKnown);

                if (Target.type == ZYDIS_OPERAND_TYPE_REGISTER && ((Info.mnemonic == ZYDIS_MNEMONIC_TEST && Source.type == ZYDIS_OPERAND_TYPE_REGISTER && Source.reg.value == Target.reg.value) || (Info.mnemonic == ZYDIS_MNEMONIC_CMP && Source.type == ZYDIS_OPERAND_TYPE_IMMEDIATE)))
                {
                    Tested = Target.reg.value;
                    TestedValue = Info.mnemonic == ZYDIS_MNEMONIC_CMP ? Right : 0;
                }

                if (Info.mnemonic != ZYDIS_MNEMONIC_CMP && Info.mnemonic != ZYDIS_MNEMONIC_TEST)
                    Set(Operation, Target, Address, Result, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_INC:
            case ZYDIS_MNEMONIC_DEC:
            case ZYDIS_MNEMONIC_NOT:
            case ZYDIS_MNEMONIC_NEG:
            case ZYDIS_MNEMONIC_BSWAP:
            {
                if (!Width || Width > 64)
                    break;

                auto bKnown = Get(Operation, Target, Address, Left);
                auto Sign = 1ull << (Width - 1);
                uint64_t Result = 0;
                Left &= Mask;

                switch (Info.mnemonic)
                {
                case ZYDIS_MNEMONIC_INC:
                    Result = (Left + 1) & Mask;
                    SetFlags(Result, Width, bCarry, Result == Sign, bKnown);
                    break;
                case ZYDIS_MNEMONIC_DEC:
                    Result = (Left - 1) & Mask;
                    SetFlags(Result, Width, bCarry, Left == Sign, bKnown);
                    break;
                case ZYDIS_MNEMONIC_NOT:
                    Result = ~Left & Mask;
                    break;
                case ZYDIS_MNEMONIC_NEG:
                    Result = (0 - Left) & Mask;
                    SetFlags(Result, Width, Left != 0, Left == Sign, bKnown);
                    break;
                default:
                    Result = Width == 64 ? _byteswap_uint64(Left) : _byteswap_ulong((uint32_t)Left);
                    break;
                }

                Set(Operation, Target, Address, Result, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_IMUL:
            {
                if (!Width || Width > 64 || Info.operand_count_visible < 2)
                    break;

                auto& Factor = Info.operand_count_visible == 3 ? Operation.Operands[2] : Source;
                auto bKnown = Get(Operation, Info.operand_count_visible == 3 ? Source : Target, Address, Left);
                bKnown = Get(Operation, Factor, Address, Right) && bKnown;
                bFlags = false;
                Tested = ZYDIS_REGISTER_NONE;
                Set(Operation, Target, Address, (Left * Right) & Mask, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_SHL:
            case ZYDIS_MNEMONIC_SHR:
            case ZYDIS_MNEMONIC_SAR:
            case ZYDIS_MNEMONIC_ROL:
            case ZYDIS_MNEMONIC_ROR:
            {
                if (!Width || Width > 64)
                    break;

                auto bKnown = Get(Operation, Target, Address, Left);

                if (Info.operand_count_visible >= 2)
                    bKnown = Get(Operation, Source, Address, Right) && bKnown;
                else
                    Right = 1;

                auto Count = (uint32_t)(Right & (Width == 64 ? 63 : 31));
                uint64_t Result = 0;
                Left &= Mask;

                switch (Info.mnemonic)
                {
                case ZYDIS_MNEMONIC_SHL:
                    Result = Count >= 64 ? 0 : (Left << Count) & Mask;
                    break;
                case ZYDIS_MNEMONIC_SHR:
                    Result = Count >= 64 ? 0 : Left >> Count;
                    break;
                case ZYDIS_MNEMONIC_SAR:
                    Result = (uint64_t)(((int64_t)(Left << (64 - Width)) >> (64 - Width)) >> Count) & Mask;
                    break;
                case ZYDIS_MNEMONIC_ROL:
                    Count %= Width;
                    Result = Count ? ((Left << Count) | (Left >> (Width - Count))) & Mask : Left;
                    break;
                default:
                    Count %= Width;
                    Result = Count ? ((Left >> Count) | (Left << (Width - Count))) & Mask : Left;
                    break;
                }

                if (Info.mnemonic == ZYDIS_MNEMONIC_ROL || Info.mnemonic == ZYDIS_MNEMONIC_ROR)
                    bFlags = bFlags && Count == 0;
                else
                    SetFlags(Result, Width, false, false, bKnown && Count);

                Set(Operation, Target, Address, Result, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_XCHG:
            {
                if (!Width || Width > 64 || Info.operand_count_visible != 2)
                    break;

                auto bLeft = Get(Operation, Target, Address, Left);
                auto bRight = Get(Operation, Source, Address, Right);
                Set(Operation, Target, Address, Right, bRight);
                Set(Operation, Source, Address, Left, bLeft);

                return;
            }
            case ZYDIS_MNEMONIC_PUSH:
            {
                auto bKnown = Get(Operation, Target, Address, Left);
                uint8_t Bytes[8];
                memcpy(Bytes, &Left, sizeof(Bytes));
                Registers[4] -= 8;
                Store(Registers[4], 8, Bytes, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_POP:
            {
                uint8_t Bytes[8] = {};
                auto bKnown = Load(Registers[4], 8, Bytes);
                memcpy(&Left, Bytes, sizeof(Left));
                Registers[4] += 8;
                Set(Operation, Target, Address, Left, bKnown);

                return;
            }
            case ZYDIS_MNEMONIC_MOVUPS:
            case ZYDIS_MNEMONIC_MOVAPS:
            case ZYDIS_MNEMONIC_MOVUPD:
            case ZYDIS_MNEMONIC_MOVAPD:
            case ZYDIS_MNEMONIC_MOVDQU:
            case ZYDIS_MNEMONIC_MOVDQA:
            case ZYDIS_MNEMONIC_LDDQU:
            case ZYDIS_MNEMONIC_VMOVUPS:
            case ZYDIS_MNEMONIC_VMOVAPS:
            case ZYDIS_MNEMONIC_VMOVDQU:
            case ZYDIS_MNEMONIC_VMOVDQA:
            {
                if (Width != 128 || Info.operand_count_visible != 2)
                    break;

                uint8_t Bytes[16];
                auto Valid = GetVector(Operation, Source, Address, 16, Bytes);
                SetVector(Operation, Target, Address, 16, Bytes, Valid, true);

                return;
            }
            case ZYDIS_MNEMONIC_MOVQ:
            case ZYDIS_MNEMONIC_MOVD:
            case ZYDIS_MNEMONIC_VMOVQ:
            case ZYDIS_MNEMONIC_VMOVD:
            case ZYDIS_MNEMONIC_MOVSD:
            case ZYDIS_MNEMONIC_MOVSS:
            {
                if (Info.operand_count_visible != 2 || (!IsVector(Target) && !IsVector(Source)))
                    break;

                auto Part = (uint32_t)std::min<uint16_t>(IsVector(Target) ? Source.size : Width, 64) / 8;
                auto bMerge = (Info.mnemonic == ZYDIS_MNEMONIC_MOVSD || Info.mnemonic == ZYDIS_MNEMONIC_MOVSS) && IsVector(Target) && IsVector(Source);
                uint8_t Bytes[16];
                auto Valid = GetVector(Operation, Source, Address, Part, Bytes);
                SetVector(Operation, Target, Address, Part, Bytes, Valid, !bMerge);

                return;
            }
            case ZYDIS_MNEMONIC_XORPS:
            case ZYDIS_MNEMONIC_PXOR:
            case ZYDIS_MNEMONIC_VXORPS:
            case ZYDIS_MNEMONIC_VPXOR:
            {
                auto& First = Info.operand_count_visible == 3 ? Source : Target;
                auto& Second = Info.operand_count_visible == 3 ? Operation.Operands[2] : Source;

                if (Width != 128 || !IsVector(Target))
                    break;

                uint8_t Bytes[16];
                uint8_t Other[16];
                auto Valid = GetVector(Operation, First, Address, 16, Bytes) & GetVector(Operation, Second, Address, 16, Other);

                if (First.type == ZYDIS_OPERAND_TYPE_REGISTER && Second.type == ZYDIS_OPERAND_TYPE_REGISTER && First.reg.value == Second.reg.value)
                    Valid = 0xFFFF;

                for (auto Index = 0; Index < 16; Index++)
                    Bytes[Index] = First.type == ZYDIS_OPERAND_TYPE_REGISTER && Second.type == ZYDIS_OPERAND_TYPE_REGISTER && First.reg.value == Second.reg.value ? 0 : Bytes[Index] ^ Other[Index];

                SetVector(Operation, Target, Address, 16, Bytes, Valid, true);

                return;
            }
            default:
                break;
            }

            Invalidate(Operation, Address);
        }

    private:
        struct FPage
        {
            uint64_t Number = 0;
            uint32_t Used = 0;
            uint32_t Stamps[0x1000] = {};
            uint8_t Bytes[0x1000] = {};
        };

        struct FUndo
        {
            uint64_t Address;
            uint32_t Stamp;
            uint8_t Byte;
        };

        struct FRegister
        {
            int8_t Slot = -1;
            int8_t Vector = -1;
            int8_t Lane = -1;
            uint8_t Width = 0;
            uint8_t Shift = 0;
        };

        static const FRegister& Describe(ZydisRegister Register)
        {
            static auto Table = []
            {
                std::array<FRegister, ZYDIS_REGISTER_MAX_VALUE + 1> Table{};

                for (int32_t Value = 0; Value <= ZYDIS_REGISTER_MAX_VALUE; Value++)
                {
                    auto Current = (ZydisRegister)Value;
                    auto Largest = ZydisRegisterGetLargestEnclosing(ZYDIS_MACHINE_MODE_LONG_64, Current);
                    auto Class = ZydisRegisterGetClass(Current);
                    auto Id = ZydisRegisterGetId(Current);
                    auto& Entry = Table[Value];

                    Entry.Width = (uint8_t)(ZydisRegisterGetWidth(ZYDIS_MACHINE_MODE_LONG_64, Current) / 8);

                    if (ZydisRegisterGetClass(Largest) == ZYDIS_REGCLASS_GPR64 && Entry.Width && Entry.Width <= 8)
                    {
                        Entry.Slot = (int8_t)ZydisRegisterGetId(Largest);
                        Entry.Shift = Current == ZYDIS_REGISTER_AH || Current == ZYDIS_REGISTER_CH || Current == ZYDIS_REGISTER_DH || Current == ZYDIS_REGISTER_BH ? 8 : 0;
                    }

                    if (Class == ZYDIS_REGCLASS_XMM && Id >= 0 && Id < 16)
                        Entry.Vector = (int8_t)Id;

                    if ((Class == ZYDIS_REGCLASS_XMM || Class == ZYDIS_REGCLASS_YMM || Class == ZYDIS_REGCLASS_ZMM) && Id >= 0 && Id < 16)
                        Entry.Lane = (int8_t)Id;
                }

                return Table;
            }();

            return Table[(uint32_t)Register < Table.size() ? Register : ZYDIS_REGISTER_NONE];
        }

        static int32_t VectorIndex(ZydisRegister Register)
        {
            return Describe(Register).Vector;
        }

        static bool IsVector(const ZydisDecodedOperand& Operand)
        {
            return Operand.type == ZYDIS_OPERAND_TYPE_REGISTER && VectorIndex(Operand.reg.value) >= 0;
        }

        bool Read(ZydisRegister Register, uint64_t& Value) const
        {
            auto& Entry = Describe(Register);

            if (Entry.Slot < 0)
                return false;

            auto Needed = (uint8_t)(((1u << Entry.Width) - 1) << (Entry.Shift / 8));

            if ((Known[Entry.Slot] & Needed) != Needed)
                return false;

            Value = (Registers[Entry.Slot] >> Entry.Shift) & Bits(Entry.Width * 8);

            return true;
        }

        void Write(ZydisRegister Register, uint64_t Value, bool bKnown)
        {
            auto& Entry = Describe(Register);

            if (Entry.Slot < 0)
            {
                if (Entry.Lane >= 0)
                    VectorKnown[Entry.Lane] = 0;

                return;
            }

            if (Entry.Width >= 4)
            {
                Registers[Entry.Slot] = Value & Bits(Entry.Width * 8);
                Known[Entry.Slot] = bKnown ? 0xFF : 0;
            }
            else
            {
                auto Field = Bits(Entry.Width * 8) << Entry.Shift;
                auto Bytes = (uint8_t)(((1u << Entry.Width) - 1) << (Entry.Shift / 8));
                Registers[Entry.Slot] = (Registers[Entry.Slot] & ~Field) | ((Value << Entry.Shift) & Field);
                Known[Entry.Slot] = bKnown ? Known[Entry.Slot] | Bytes : Known[Entry.Slot] & ~Bytes;
            }

            if (Entry.Slot == 4 && !bKnown)
                SetStackPointer(0x400000000000 + ((uint64_t)Generation << 20));

            if (Tested != ZYDIS_REGISTER_NONE && Describe(Tested).Slot == Entry.Slot)
                Tested = ZYDIS_REGISTER_NONE;

            Aliases[Entry.Slot] = 0;
        }

        bool IsConstant(uint64_t Address) const
        {
            auto Offset = Address - Image;

            if (Offset >= Size)
                return false;

            for (auto [Begin, End] : Constants)
            {
                if (Offset >= Begin && Offset < End)
                    return true;
            }

            return false;
        }

        FPage* Page(uint64_t Address, bool bCreate)
        {
            auto Number = Address >> 12;

            if (Recent && Recent->Number == Number && Recent->Used == Generation)
                return Recent;

            FPage* Free = nullptr;

            for (auto& Entry : Pages)
            {
                if (Entry->Used != Generation)
                {
                    Free = Free ? Free : Entry.get();
                    continue;
                }

                if (Entry->Number == Number)
                    return Recent = Entry.get();
            }

            if (!bCreate)
                return nullptr;

            if (!Free)
            {
                if (Pages.size() >= 64)
                    return nullptr;

                Pages.push_back(std::make_unique<FPage>());
                Free = Pages.back().get();
            }

            Free->Number = Number;
            Free->Used = Generation;

            return Recent = Free;
        }

        bool Load(uint64_t Address, uint32_t Count, uint8_t* Bytes)
        {
            for (uint32_t Offset = 0; Offset < Count; Offset++)
            {
                auto Byte = Address + Offset;

                if (IsConstant(Byte))
                {
                    Bytes[Offset] = *(const uint8_t*)Byte;
                    continue;
                }

                auto Found = Page(Byte, false);

                if (!Found || Found->Stamps[Byte & 0xFFF] != Generation)
                    return false;

                Bytes[Offset] = Found->Bytes[Byte & 0xFFF];
            }

            return true;
        }

        void Store(uint64_t Address, uint32_t Count, const uint8_t* Bytes, uint32_t Valid)
        {
            for (uint32_t Offset = 0; Offset < Count; Offset++)
            {
                auto Byte = Address + Offset;
                auto bKnown = Offset < 32 && ((Valid >> Offset) & 1);

                if (IsConstant(Byte))
                    continue;

                if (auto Found = Page(Byte, bKnown))
                {
                    if (bCopy)
                        Undo.push_back({ Byte, Found->Stamps[Byte & 0xFFF], Found->Bytes[Byte & 0xFFF] });

                    Found->Stamps[Byte & 0xFFF] = bKnown ? Generation : 0;
                    Found->Bytes[Byte & 0xFFF] = Bytes[Offset];
                }
            }

            if (bRecord && Count)
            {
                Low = std::min<uint64_t>(Low, Address);
                High = std::max<uint64_t>(High, Address + Count - 1);
                Stores.emplace_back(Address, Count);
            }

            if (bCopy && Count && Address - Image < Size)
                CopyLow = std::min<uint64_t>(CopyLow, Address);
        }

        void Store(uint64_t Address, uint32_t Count, const uint8_t* Bytes, bool bKnown)
        {
            Store(Address, Count, Bytes, bKnown ? 0xFFFFu : 0u);
        }

        bool Effective(const FOperation& Operation, const ZydisDecodedOperand& Operand, const uint8_t* Address, uint64_t& Result) const
        {
            if (Operand.type != ZYDIS_OPERAND_TYPE_MEMORY || Operand.mem.segment == ZYDIS_REGISTER_FS || Operand.mem.segment == ZYDIS_REGISTER_GS)
                return false;

            uint64_t Value = 0;
            uint64_t Part = 0;

            if (Operand.mem.base == ZYDIS_REGISTER_RIP)
                Value = (uint64_t)Address + Operation.Info.length;
            else if (Operand.mem.base != ZYDIS_REGISTER_NONE)
            {
                if (!Read(Operand.mem.base, Part))
                    return false;

                Value = Part;
            }

            if (Operand.mem.index != ZYDIS_REGISTER_NONE)
            {
                if (!Read(Operand.mem.index, Part))
                    return false;

                Value += Part * Operand.mem.scale;
            }

            Result = Value + (uint64_t)Operand.mem.disp.value;

            return true;
        }

        bool Get(const FOperation& Operation, const ZydisDecodedOperand& Operand, const uint8_t* Address, uint64_t& Value)
        {
            Value = 0;

            switch (Operand.type)
            {
            case ZYDIS_OPERAND_TYPE_REGISTER:
                return Read(Operand.reg.value, Value);
            case ZYDIS_OPERAND_TYPE_IMMEDIATE:
                Value = Operand.imm.value.u;
                return true;
            case ZYDIS_OPERAND_TYPE_MEMORY:
            {
                uint64_t Target = 0;
                uint8_t Bytes[8] = {};

                if (!Operand.size || Operand.size > 64 || !Effective(Operation, Operand, Address, Target) || !Load(Target, Operand.size / 8, Bytes))
                    return false;

                memcpy(&Value, Bytes, sizeof(Value));

                return true;
            }
            default:
                return false;
            }
        }

        void Set(const FOperation& Operation, const ZydisDecodedOperand& Operand, const uint8_t* Address, uint64_t Value, bool bKnown)
        {
            if (Operand.type == ZYDIS_OPERAND_TYPE_REGISTER)
                Write(Operand.reg.value, Value, bKnown);
            else if (uint64_t Target = 0; Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && Operand.size && Operand.size <= 64 && Effective(Operation, Operand, Address, Target))
            {
                uint8_t Bytes[8];
                memcpy(Bytes, &Value, sizeof(Bytes));
                Store(Target, Operand.size / 8, Bytes, bKnown);
            }
        }

        uint32_t GetVector(const FOperation& Operation, const ZydisDecodedOperand& Operand, const uint8_t* Address, uint32_t Count, uint8_t* Bytes)
        {
            memset(Bytes, 0, 16);

            if (auto Vector = Operand.type == ZYDIS_OPERAND_TYPE_REGISTER ? VectorIndex(Operand.reg.value) : -1; Vector >= 0)
            {
                memcpy(Bytes, Vectors[Vector], 16);

                return VectorKnown[Vector] & ((1u << Count) - 1);
            }

            if (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY)
            {
                uint64_t Target = 0;

                return Effective(Operation, Operand, Address, Target) && Load(Target, Count, Bytes) ? (1u << Count) - 1 : 0;
            }

            uint64_t Value = 0;
            auto bKnown = Get(Operation, Operand, Address, Value);
            memcpy(Bytes, &Value, std::min<uint32_t>(Count, 8));

            return bKnown ? (1u << Count) - 1 : 0;
        }

        void SetVector(const FOperation& Operation, const ZydisDecodedOperand& Operand, const uint8_t* Address, uint32_t Count, const uint8_t* Bytes, uint32_t Valid, bool bZeroUpper)
        {
            if (auto Vector = Operand.type == ZYDIS_OPERAND_TYPE_REGISTER ? VectorIndex(Operand.reg.value) : -1; Vector >= 0)
            {
                auto Part = (1u << Count) - 1;

                for (uint32_t Offset = 0; Offset < 16; Offset++)
                    Vectors[Vector][Offset] = Offset < Count ? Bytes[Offset] : bZeroUpper ? 0 : Vectors[Vector][Offset];

                VectorKnown[Vector] = (uint16_t)((Valid & Part) | (bZeroUpper ? ~Part & 0xFFFF : VectorKnown[Vector] & ~Part));

                return;
            }

            if (Operand.type == ZYDIS_OPERAND_TYPE_MEMORY)
            {
                if (uint64_t Target = 0; Effective(Operation, Operand, Address, Target))
                    Store(Target, Count, Bytes, Valid);

                return;
            }

            uint64_t Value = 0;
            memcpy(&Value, Bytes, std::min<uint32_t>(Count, 8));
            Set(Operation, Operand, Address, Value, (Valid & ((1u << std::min<uint32_t>(Count, 8)) - 1)) == ((1u << std::min<uint32_t>(Count, 8)) - 1));
        }

        void SetFlags(uint64_t Result, uint32_t Width, bool bCarryOut, bool bOverflowOut, bool bKnown)
        {
            Tested = ZYDIS_REGISTER_NONE;
            bFlags = bKnown;
            bZero = (Result & Bits(Width)) == 0;
            bSign = (Result >> (Width - 1)) & 1;
            bCarry = bCarryOut;
            bOverflow = bOverflowOut;
        }

        void Invalidate(const FOperation& Operation, const uint8_t* Address)
        {
            for (ZyanU8 Slot = 0; Slot < Operation.Info.operand_count; Slot++)
            {
                auto& Operand = Operation.Operands[Slot];

                if (!(Operand.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE))
                    continue;

                if (Operand.type == ZYDIS_OPERAND_TYPE_REGISTER)
                    Write(Operand.reg.value, 0, false);
                else if (uint64_t Target = 0; Operand.type == ZYDIS_OPERAND_TYPE_MEMORY && Effective(Operation, Operand, Address, Target))
                {
                    uint8_t Bytes[64] = {};
                    Store(Target, std::min<uint32_t>(Operand.size / 8, 64), Bytes, false);
                }
            }

            bFlags = false;
            Tested = ZYDIS_REGISTER_NONE;
        }

        uint64_t Registers[16] = {};
        uint8_t Known[16] = {};
        uint8_t Vectors[16][16] = {};
        uint16_t VectorKnown[16] = {};
        FState Saved = {};
        uint32_t Aliases[16] = {};
        uint32_t Alias = 0;
        ZydisRegister Tested = ZYDIS_REGISTER_NONE;
        uint64_t TestedValue = 0;
        bool bFlags = false;
        bool bRecord = false;
        bool bCopy = false;
        uint64_t CopyLow = ~0ull;
        std::vector<std::pair<uint64_t, uint32_t>> Stores;
        std::vector<FUndo> Undo;
        bool bZero = false;
        bool bSign = false;
        bool bCarry = false;
        bool bOverflow = false;
        uint64_t Low = ~0ull;
        uint64_t High = 0;
        uint32_t Generation = 0;
        uint64_t Image = 0;
        uint64_t Size = 0;
        std::vector<std::pair<uint64_t, uint64_t>> Constants;
        std::vector<std::unique_ptr<FPage>> Pages;
        FPage* Recent = nullptr;
    };

    static void Call(FMachine& Machine, const uint8_t* Function)
    {
        FMachine::FState Before;
        FOperation Operation;

        Machine.Capture(Before);
        Machine.SetStackPointer(Machine.StackPointer() - 8);

        for (int32_t Steps = 0; Function && Steps < 2048 && InCode(Function, 0, ZYDIS_MAX_INSTRUCTION_LENGTH) && Decode(Function, Operation); Steps++)
        {
            auto Category = Operation.Info.meta.category;

            if (Category == ZYDIS_CATEGORY_RET)
            {
                Machine.Preserve(Before);

                return;
            }

            if (Category == ZYDIS_CATEGORY_UNCOND_BR && Operation.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                Function = Private::Unhooked(Function + Operation.Info.length + Operation.Operands[0].imm.value.s);
                continue;
            }

            if (Category == ZYDIS_CATEGORY_COND_BR || Category == ZYDIS_CATEGORY_UNCOND_BR || Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_SYSTEM || Category == ZYDIS_CATEGORY_INTERRUPT)
                break;

            Machine.Execute(Operation, Function);
            Function += Operation.Info.length;
        }

        Machine.Preserve(Before);
        Machine.Clobber();
    }

    static const uint8_t* Synchronize(const uint8_t* Begin, const uint8_t* End, const uint8_t** Previous)
    {
        ZydisDecodedInstruction Info;

        for (auto Start = Begin; Start < Begin + 16 && Start < End; Start++)
        {
            const uint8_t* Last = nullptr;
            auto Address = Start;

            while (Address < End && Decode(Address, Info))
            {
                Last = Address;
                Address += Info.length;
            }

            if (Address == End)
            {
                if (Previous)
                    *Previous = Last;

                return Start;
            }
        }

        return nullptr;
    }

    static bool IsEpilogue(const FOperation& Operation)
    {
        auto& Target = Operation.Operands[0];
        auto& Source = Operation.Operands[1];
        auto Mnemonic = Operation.Info.mnemonic;

        if (Mnemonic == ZYDIS_MNEMONIC_POP)
            return true;

        if (Target.type != ZYDIS_OPERAND_TYPE_REGISTER)
            return false;

        if (Target.reg.value == ZYDIS_REGISTER_RSP && (Mnemonic == ZYDIS_MNEMONIC_ADD || Mnemonic == ZYDIS_MNEMONIC_SUB || Mnemonic == ZYDIS_MNEMONIC_LEA || Mnemonic == ZYDIS_MNEMONIC_MOV))
            return true;

        if (Source.type != ZYDIS_OPERAND_TYPE_MEMORY || (Source.mem.base != ZYDIS_REGISTER_RSP && Source.mem.base != ZYDIS_REGISTER_R11))
            return false;

        if (Mnemonic == ZYDIS_MNEMONIC_LEA)
            return Target.reg.value == ZYDIS_REGISTER_R11;

        switch (Target.reg.value)
        {
        case ZYDIS_REGISTER_RBX:
        case ZYDIS_REGISTER_RBP:
        case ZYDIS_REGISTER_RSI:
        case ZYDIS_REGISTER_RDI:
        case ZYDIS_REGISTER_R12:
        case ZYDIS_REGISTER_R13:
        case ZYDIS_REGISTER_R14:
        case ZYDIS_REGISTER_R15:
            return Mnemonic == ZYDIS_MNEMONIC_MOV;
        default:
            return ZydisRegisterGetClass(Target.reg.value) == ZYDIS_REGCLASS_XMM && ZydisRegisterGetId(Target.reg.value) >= 6 && (Mnemonic == ZYDIS_MNEMONIC_MOVAPS || Mnemonic == ZYDIS_MNEMONIC_MOVUPS || Mnemonic == ZYDIS_MNEMONIC_MOVDQA || Mnemonic == ZYDIS_MNEMONIC_MOVDQU);
        }
    }

    static const uint8_t* Step(FMachine& Machine, const FOperation& Operation, const uint8_t* Address, const uint8_t* Begin, int32_t Steps, bool& bBody)
    {
        auto Next = Address + Operation.Info.length;
        auto Category = Operation.Info.meta.category;

        auto bEpilogue = Category == ZYDIS_CATEGORY_RET || IsEpilogue(Operation);

        if (bBody && bEpilogue)
            Machine.Save();

        bBody = !bEpilogue;

        if (Category == ZYDIS_CATEGORY_COND_BR)
        {
            auto Target = Next + Operation.Operands[0].imm.value.s;
            bool bTaken = false;

            if (Target < Address && Target >= Begin && Target + 0x200 > Address && Steps < 0x8000 && Machine.Condition(Operation.Info.mnemonic, bTaken) && bTaken)
                return Target;

            Machine.Refine(Operation.Info.mnemonic, false);

            return Next;
        }

        if (Category == ZYDIS_CATEGORY_CALL)
        {
            if (Operation.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
                Call(Machine, Private::Unhooked(Next + Operation.Operands[0].imm.value.s));
            else
                Machine.Clobber();
        }
        else if (Category == ZYDIS_CATEGORY_RET)
            Machine.Restore();
        else if (Category != ZYDIS_CATEGORY_UNCOND_BR)
            Machine.Execute(Operation, Address);

        return Next;
    }

    static bool Prepare(FMachine& Machine, const uint8_t* Begin, const uint8_t* End)
    {
        FOperation Operation;
        auto bBody = true;
        int32_t Steps = 0;

        for (auto Address = Begin; Address < End; Steps++)
        {
            if (Steps >= 0x40000 || !Decode(Address, Operation))
                return false;

            Address = Step(Machine, Operation, Address, Begin, Steps, bBody);
        }

        return true;
    }

    static bool IsDecryption(const uint8_t* Head, const uint8_t* Branch)
    {
        ZydisDecodedInstruction Info;
        auto Address = Head;
        auto bMixed = false;
        auto bStep = false;

        while (Address < Branch && Decode(Address, Info))
        {
            auto Category = Info.meta.category;
            auto Mnemonic = Info.mnemonic;
            auto Immediate = Info.raw.imm[0].size ? (int64_t)Info.raw.imm[0].value.s : 0;

            if (Category == ZYDIS_CATEGORY_CALL || Category == ZYDIS_CATEGORY_RET || Category == ZYDIS_CATEGORY_SYSTEM || Category == ZYDIS_CATEGORY_INTERRUPT)
                return false;

            bMixed |= Mnemonic == ZYDIS_MNEMONIC_XOR && (Info.attributes & ZYDIS_ATTRIB_HAS_MODRM) && Info.raw.modrm.mod != 3;
            bStep |= Mnemonic == ZYDIS_MNEMONIC_IMUL || Mnemonic == ZYDIS_MNEMONIC_SHL || Mnemonic == ZYDIS_MNEMONIC_SHR || Mnemonic == ZYDIS_MNEMONIC_ROL || Mnemonic == ZYDIS_MNEMONIC_ROR || ((Mnemonic == ZYDIS_MNEMONIC_ADD || Mnemonic == ZYDIS_MNEMONIC_SUB || Mnemonic == ZYDIS_MNEMONIC_XOR) && (Immediate >= 0x100 || Immediate <= -0x100));
            Address += Info.length;
        }

        return Address == Branch && bMixed && bStep && Decode(Branch, Info) && Info.meta.category == ZYDIS_CATEGORY_COND_BR;
    }

    static bool Run(FMachine& Machine, std::vector<std::pair<const uint8_t*, FOperation>>& Body, const uint8_t* Head, const uint8_t* Branch, std::string& Text)
    {
        if (Body.empty() || Body.front().first != Head || Body.back().first != Branch)
        {
            Body.clear();

            for (auto Address = Head; Address <= Branch && Body.size() < 512; )
            {
                Body.emplace_back(Address, FOperation());

                auto& Operation = Body.back().second;

                if (!Decode(Address, Operation))
                {
                    Body.clear();

                    return false;
                }

                Address += Operation.Info.length;
            }

            if (Body.empty() || Body.back().first != Branch)
            {
                Body.clear();

                return false;
            }
        }

        auto Address = Head;
        auto bDone = false;

        Machine.Record(true);

        for (int32_t Steps = 0; Steps < 0x20000 && (uint64_t)Steps < 0x100 + Machine.Written() * 0x40 && !Machine.Overflowed(); Steps++)
        {
            if (Address < Head || Address > Branch)
            {
                bDone = true;
                break;
            }

            auto Found = std::lower_bound(Body.begin(), Body.end(), Address, [](const auto& Item, const uint8_t* Value) { return Item.first < Value; });

            if (Found == Body.end() || Found->first != Address)
                break;

            auto& Operation = Found->second;
            auto Next = Address + Operation.Info.length;
            auto Category = Operation.Info.meta.category;

            if (Category == ZYDIS_CATEGORY_COND_BR)
            {
                bool bTaken = false;

                if (!Machine.Condition(Operation.Info.mnemonic, bTaken))
                    break;

                Address = bTaken ? Next + Operation.Operands[0].imm.value.s : Next;
                continue;
            }

            if (Category == ZYDIS_CATEGORY_UNCOND_BR)
            {
                if (Operation.Operands[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
                    break;

                Address = Next + Operation.Operands[0].imm.value.s;
                continue;
            }

            Machine.Execute(Operation, Address);
            Address = Next;
        }

        Machine.Record(false);

        return bDone && Machine.Recorded(Text);
    }

    static void Tail(FMachine& Machine, const uint8_t* Branch, const std::string& Text, uint32_t& Copy)
    {
        FOperation Operation;
        auto Address = Decode(Branch, Operation) ? Branch + Operation.Info.length : Branch;
        FMachine::FContext Context;

        Machine.Capture(Context);
        Machine.Copy(true);

        for (int32_t Steps = 0; Steps < 64 && Decode(Address, Operation); Steps++)
        {
            switch (Operation.Info.mnemonic)
            {
            case ZYDIS_MNEMONIC_MOV:
            case ZYDIS_MNEMONIC_MOVZX:
            case ZYDIS_MNEMONIC_MOVSX:
            case ZYDIS_MNEMONIC_MOVSXD:
            case ZYDIS_MNEMONIC_LEA:
            case ZYDIS_MNEMONIC_MOVUPS:
            case ZYDIS_MNEMONIC_MOVAPS:
            case ZYDIS_MNEMONIC_MOVDQU:
            case ZYDIS_MNEMONIC_MOVDQA:
            case ZYDIS_MNEMONIC_MOVSD:
            case ZYDIS_MNEMONIC_MOVSS:
            case ZYDIS_MNEMONIC_MOVQ:
            case ZYDIS_MNEMONIC_MOVD:
            case ZYDIS_MNEMONIC_VMOVUPS:
            case ZYDIS_MNEMONIC_VMOVAPS:
            case ZYDIS_MNEMONIC_VMOVDQU:
            case ZYDIS_MNEMONIC_VMOVDQA:
                Machine.Execute(Operation, Address);
                Address += Operation.Info.length;
                continue;
            default:
                break;
            }

            break;
        }

        Machine.Copy(false);
        Copy = Machine.Copied(Text);
        Machine.Rollback();
        Machine.Apply(Context);
    }

    static bool Decrypt(FMachine& Machine, std::vector<std::pair<const uint8_t*, FOperation>>& Body, const uint8_t* Head, const uint8_t* Branch, std::string& Text, uint32_t& Copy)
    {
        DWORD64 ImageBase = 0;
        ZydisDecodedInstruction Info;
        const uint8_t* Previous = nullptr;
        std::vector<const uint8_t*> Entries;
        auto Chunk = RtlLookupFunctionEntry((DWORD64)Head, &ImageBase, nullptr);
        auto Floor = Chunk ? Base() + Chunk->BeginAddress : Head - 0x100;
        auto Begin = std::max<const uint8_t*>(Floor, Head - 0x100);
        auto Start = Synchronize(Begin, Head, &Previous);

        if (!Start || (Begin == Floor && Start != Floor))
            return false;

        if (!Previous || !Decode(Previous, Info) || (Info.meta.category != ZYDIS_CATEGORY_UNCOND_BR && Info.meta.category != ZYDIS_CATEGORY_RET && Info.mnemonic != ZYDIS_MNEMONIC_INT3))
            Entries.push_back(Head);
        else
        {
            auto [RangeBegin, RangeEnd] = FindRange(Head);

            for (auto Address = RangeBegin; Address && Address + 2 <= RangeEnd && Entries.size() < 4; Address++)
            {
                if (Address == Branch)
                    continue;

                if ((Address[0] == 0xE9 && Address + 5 <= RangeEnd && Address + 5 + *(int32_t*)(Address + 1) == Head) || (Address[0] == 0x0F && (Address[1] & 0xF0) == 0x80 && Address + 6 <= RangeEnd && Address + 6 + *(int32_t*)(Address + 2) == Head) || ((Address[0] == 0xEB || (Address[0] & 0xF0) == 0x70) && Address + 2 + (int8_t)Address[1] == Head))
                    Entries.push_back(Address);
            }
        }

        for (auto Entry : Entries)
        {
            auto EntryBegin = Start;

            if (Entry != Head)
            {
                auto EntryChunk = RtlLookupFunctionEntry((DWORD64)Entry, &ImageBase, nullptr);
                auto EntryFloor = EntryChunk ? Base() + EntryChunk->BeginAddress : Entry - 0x100;
                auto EntryStart = std::max<const uint8_t*>(EntryFloor, Entry - 0x100);
                EntryBegin = EntryStart == EntryFloor ? EntryStart : Synchronize(EntryStart, Entry, nullptr);
            }

            if (!EntryBegin)
                continue;

            Machine.Reset();

            if (Prepare(Machine, EntryBegin, Entry) && Run(Machine, Body, Head, Branch, Text))
            {
                Tail(Machine, Branch, Text, Copy);

                return true;
            }
        }

        return false;
    }

    static void Sweep(FMachine& Machine, std::vector<std::pair<const uint8_t*, FOperation>>& Body, const uint8_t* Begin, const uint8_t* End, std::vector<std::pair<const uint8_t*, const uint8_t*>>& Loops, std::vector<std::tuple<std::string, uint32_t, uint32_t>>& Table)
    {
        FOperation Operation;
        ZydisDecodedInstruction Info;
        std::string Text;
        uint32_t Copy = 0;
        std::unordered_map<const uint8_t*, FMachine::FState> Entries;
        std::unordered_set<const uint8_t*> Done;
        auto bBody = true;
        auto bFlow = true;

        std::sort(Loops.begin(), Loops.end());
        Machine.Reset();

        for (auto [Address, Steps] = std::pair(Begin, 0); Address < End && Steps < 0x100000; Steps++)
        {
            if (!bFlow)
            {
                if (auto Entry = Entries.find(Address); Entry != Entries.end())
                    Machine.Apply(Entry->second);
            }

            auto Found = std::lower_bound(Loops.begin(), Loops.end(), std::pair(Address, (const uint8_t*)nullptr));

            if (Found != Loops.end() && Found->first == Address)
            {
                if (!Decode(Found->second, Info))
                    return;

                Address = Found->second + Info.length;

                if (bFlow && !Done.contains(Found->first) && Run(Machine, Body, Found->first, Found->second, Text))
                {
                    Tail(Machine, Found->second, Text, Copy);
                    Table.emplace_back(Text, (uint32_t)(Found->second - Base()), Copy);
                    Done.insert(Found->first);
                }

                bFlow = true;
                continue;
            }

            if (!Decode(Address, Operation))
                return;

            auto Category = Operation.Info.meta.category;

            if ((Category == ZYDIS_CATEGORY_UNCOND_BR || Category == ZYDIS_CATEGORY_COND_BR) && Operation.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                auto Target = Address + Operation.Info.length + Operation.Operands[0].imm.value.s;

                if (Target > Address && Target < End && !Entries.contains(Target))
                    Machine.Capture(Entries[Target]);
            }

            if (Category == ZYDIS_CATEGORY_UNCOND_BR && Operation.Operands[0].type == ZYDIS_OPERAND_TYPE_IMMEDIATE)
            {
                auto Target = Address + Operation.Info.length + Operation.Operands[0].imm.value.s;
                auto Jumped = std::lower_bound(Loops.begin(), Loops.end(), std::pair(Target, (const uint8_t*)nullptr));

                if (Jumped != Loops.end() && Jumped->first == Target && !Done.contains(Target) && Run(Machine, Body, Jumped->first, Jumped->second, Text))
                {
                    Tail(Machine, Jumped->second, Text, Copy);
                    Table.emplace_back(Text, (uint32_t)(Jumped->second - Base()), Copy);
                    Done.insert(Target);
                }
            }

            bFlow = Category != ZYDIS_CATEGORY_UNCOND_BR && Category != ZYDIS_CATEGORY_RET && Operation.Info.mnemonic != ZYDIS_MNEMONIC_INT3;
            Address = Step(Machine, Operation, Address, Begin, Steps, bBody);
        }
    }

    static const std::vector<std::tuple<std::string, uint32_t, uint32_t>>& Decrypted()
    {
        static auto Table = []
        {
            std::vector<std::tuple<std::string, uint32_t, uint32_t>> Table;
            std::vector<std::tuple<const uint8_t*, const uint8_t*, const uint8_t*, const uint8_t*>> Failed;
            std::vector<std::pair<const uint8_t*, FOperation>> Body;
            std::string Text;
            uint32_t Copy = 0;
            FMachine Machine;
            DWORD64 ImageBase = 0;
            auto Image = Base();

            Body.reserve(512);

            for (auto& Section : Sections())
            {
                if (!(Section.Characteristics & IMAGE_SCN_MEM_EXECUTE))
                    continue;

                auto Begin = Image + Section.VirtualAddress;
                auto End = Begin + Section.Misc.VirtualSize;
                const uint8_t* Marker = nullptr;

                for (auto Address = Begin; Address + 6 <= End; Address++)
                {
                    const uint8_t* Head = nullptr;

                    if ((Address[0] & 0xF0) == 0x70 && (int8_t)Address[1] < 0)
                        Head = Address + 2 + (int8_t)Address[1];
                    else if (Address[0] == 0x0F && (Address[1] & 0xF0) == 0x80 && *(int32_t*)(Address + 2) < 0 && *(int32_t*)(Address + 2) >= -0x1000)
                        Head = Address + 6 + *(int32_t*)(Address + 2);

                    if (Head && Head >= Begin && Marker && Marker >= Head && Marker + 1 < Address && IsDecryption(Head, Address))
                    {
                        if (Copy = 0; Decrypt(Machine, Body, Head, Address, Text, Copy))
                            Table.emplace_back(Text, (uint32_t)(Address - Image), Copy);
                        else if (auto Chunk = RtlLookupFunctionEntry((DWORD64)Head, &ImageBase, nullptr))
                            Failed.emplace_back(Image + Chunk->BeginAddress, Image + Chunk->EndAddress, Head, Address);
                    }

                    if ((Address[0] & 0xFC) == 0x30 && (Address[1] >> 6) != 3)
                        Marker = Address;
                }
            }

            std::sort(Failed.begin(), Failed.end());

            for (size_t Index = 0; Index < Failed.size(); )
            {
                auto [Begin, End, Head, Branch] = Failed[Index];
                std::vector<std::pair<const uint8_t*, const uint8_t*>> Loops;
                auto Last = Branch;

                for (; Index < Failed.size() && std::get<0>(Failed[Index]) == Begin; Index++)
                {
                    Loops.emplace_back(std::get<2>(Failed[Index]), std::get<3>(Failed[Index]));
                    Last = std::max<const uint8_t*>(Last, std::get<3>(Failed[Index]));
                }

                Sweep(Machine, Body, Begin, std::min<const uint8_t*>({ End, Last + 0x2000, Begin + 0x80000 }), Loops, Table);
            }

            std::sort(Table.begin(), Table.end());
            Table.erase(std::unique(Table.begin(), Table.end()), Table.end());

            return Table;
        }();

        return Table;
    }

    static const uint8_t* FindCopyReference(uint32_t Copy)
    {
        static auto Slots = []
        {
            std::vector<uint64_t> Slots;
            std::vector<std::pair<uint64_t, uint64_t>> Writable;
            auto Image = Base();
            auto Size = ImageSize();

            for (auto& Section : Sections())
            {
                if (Section.Characteristics & IMAGE_SCN_MEM_WRITE)
                    Writable.emplace_back(Section.VirtualAddress, Section.VirtualAddress + Section.Misc.VirtualSize);
            }

            for (auto& Section : Sections())
            {
                if (Section.Characteristics & IMAGE_SCN_MEM_EXECUTE)
                    continue;

                for (uint64_t Offset = (Section.VirtualAddress + 7) & ~7ull; Offset + 8 <= Section.VirtualAddress + Section.Misc.VirtualSize; Offset += 8)
                {
                    auto Value = *(const uint64_t*)(Image + Offset) - (uint64_t)Image;

                    if (Value < Size && std::any_of(Writable.begin(), Writable.end(), [Value](auto& Range) { return Value >= Range.first && Value < Range.second; }))
                        Slots.push_back(Value << 32 | Offset);
                }
            }

            std::sort(Slots.begin(), Slots.end());

            return Slots;
        }();

        auto Reference = FindReference(Base() + Copy);

        for (auto Entry = std::lower_bound(Slots.begin(), Slots.end(), (uint64_t)Copy << 32); !Reference && Entry != Slots.end() && (*Entry >> 32) == Copy; Entry++)
            Reference = FindReference(Base() + (uint32_t)*Entry);

        return Reference && (Reference[-1] & 0xFB) == 0x48 ? Reference - 1 : Reference;
    }

    template<typename VisitorType>
    static void VisitDecrypted(const std::string& Text, size_t Terminator, bool bPrefix, VisitorType Visitor)
    {
        auto& Table = Decrypted();

        for (auto Entry = std::lower_bound(Table.begin(), Table.end(), std::make_tuple(Text, 0u, 0u)); Entry != Table.end() && !std::get<0>(*Entry).compare(0, Text.size(), Text); Entry++)
        {
            auto& [Plain, Branch, Copy] = *Entry;

            if (bPrefix || Plain.size() == Text.size() || (Plain.size() >= Text.size() + Terminator && Plain.find_first_not_of('\0', Text.size()) >= Text.size() + Terminator))
            {
                auto Site = Copy ? FindCopyReference(Copy) : nullptr;
                Visitor(Site ? Site : Base() + Branch);
            }
        }
    }

    static const uint8_t* FindDecrypted(const std::string& Text, size_t Terminator, bool bPrefix)
    {
        const uint8_t* Found = nullptr;

        VisitDecrypted(Text, Terminator, bPrefix, [&](const uint8_t* Site)
        {
            Found = Found && Found < Site ? Found : Site;
        });

        return Found;
    }

    static const uint8_t* FindDecrypted(const wchar_t* Text)
    {
        auto Length = wcslen(Text);
        auto bPrefix = Length && Text[Length - 1] == L'*';
        auto Size = bPrefix ? Length - 1 : Length;
        auto Found = FindDecrypted(std::string((const char*)Text, Size * sizeof(wchar_t)), sizeof(wchar_t), bPrefix);

        if (auto Narrow = NarrowText(Text); !Found && !Narrow.empty())
            Found = FindDecrypted(Narrow.substr(0, Size), 1, bPrefix);

        return Found;
    }

    static const uint8_t* FindDecrypted(const char* Text)
    {
        auto Length = strlen(Text);
        auto bPrefix = Length && Text[Length - 1] == '*';

        return FindDecrypted(std::string(Text, bPrefix ? Length - 1 : Length), 1, bPrefix);
    }

    FCode String(const wchar_t* Text)
    {
        auto Reference = FindStringReference(Text);

        if (!Reference)
        {
            if (auto Narrow = NarrowText(Text); !Narrow.empty())
            {
                if (auto Found = FindText(Narrow, nullptr))
                    Reference = FindReference(Found);
            }
        }

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        if (!Reference)
            Reference = FindDecrypted(Text);

        return FCode(FindLogCaller(Reference));
    }

    FCode String(const char* Text)
    {
        auto Reference = FindStringReference(Text);

        if (Reference && (Reference[-1] & 0xFB) == 0x48)
            Reference--;

        if (!Reference)
            Reference = FindDecrypted(Text);

        return FCode(FindLogCaller(Reference));
    }

    FCode Smallest(const wchar_t* Text)
    {
        auto Size = (wcslen(Text) + 1) * sizeof(wchar_t);
        const uint8_t* Found = nullptr;
        int64_t FoundSize = 0;

        auto Image = Base();
        auto& Table = References()[1];

        for (auto Address : FindWideTexts(Text, Size))
        {
            for (auto Item = std::lower_bound(Table.begin(), Table.end(), (uint64_t)(Address - Image) << 32); Item != Table.end() && (*Item >> 32) == (uint64_t)(Address - Image); Item++)
            {
                auto Reference = Image + (uint32_t)*Item;

                if (std::any_of(Sections().begin(), Sections().end(), [&](auto& Section) { return Reference == Image + Section.VirtualAddress; }))
                    continue;

                auto Entry = FindFunction(Reference);

                if (!Entry)
                    continue;

                auto EntrySize = FindFunctionEnd(Entry) - (Image + Entry->BeginAddress);

                if (!Found || EntrySize < FoundSize)
                {
                    Found = (Reference[-1] & 0xFB) == 0x48 ? Reference - 1 : Reference;
                    FoundSize = EntrySize;
                }
            }
        }

        if (!Found)
        {
            VisitDecrypted(std::string((const char*)Text, Size - sizeof(wchar_t)), sizeof(wchar_t), false, [&](const uint8_t* Site)
            {
                auto Entry = FindFunction(Site);

                if (!Entry)
                    return;

                auto EntrySize = FindFunctionEnd(Entry) - (Image + Entry->BeginAddress);

                if (!Found || EntrySize < FoundSize)
                {
                    Found = Site;
                    FoundSize = EntrySize;
                }
            });
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
        auto bFound = false;

        for (auto Address : FindWideTexts(Text, Size))
        {
            bFound = true;

            if (auto Code = Reference(Address, Instructions); Code.Get())
                return Code;
        }

        auto Narrow = NarrowText(Text);

        if (Narrow.empty() || (bFound && FindStringReference(Text)))
            return FCode();

        for (auto Found = FindText(Narrow, nullptr); Found; Found = FindText(Narrow, Found))
        {
            if (auto Code = Reference(Found, Instructions); Code.Get())
                return Code;
        }

        return FCode();
    }
}
