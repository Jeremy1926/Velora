#pragma once

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

    inline const wchar_t* FindString(const wchar_t* String)
    {
        auto Size = (wcslen(String) + 1) * sizeof(wchar_t);

        for (auto& Section : Sections())
        {
            auto Begin = Base() + Section.VirtualAddress;
            auto End = Begin + Section.Misc.VirtualSize;

            for (auto Address = Begin + sizeof(wchar_t); Address + Size <= End; Address += sizeof(wchar_t))
            {
                if (*(wchar_t*)Address == *String && *(wchar_t*)(Address - sizeof(wchar_t)) == 0 && !memcmp(Address, String, Size))
                    return (const wchar_t*)Address;
            }
        }

        return nullptr;
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
                    return Address;
            }
        }

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
}
