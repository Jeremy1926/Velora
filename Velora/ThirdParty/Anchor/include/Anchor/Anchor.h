#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <string_view>
#include <utility>

namespace Anchor
{
    enum class ELogLevel : uint8_t
    {
        Log,
        Error
    };

    struct FHost
    {
        void (*Log)(ELogLevel Level, const wchar_t* Message) = nullptr;
        void** (*FindVTable)(const wchar_t* Name) = nullptr;
    };

    void SetHost(const FHost& InHost);

    uint8_t* Base();

    bool IsCode(const void* Address);

    std::pair<const uint8_t*, const uint8_t*> Range(const void* Address);

    inline bool IsSlot(int64_t Value)
    {
        return Value > 0 && Value < 0x10000 && !(Value % sizeof(void*));
    }

    class FCode
    {
    public:
        FCode(const uint8_t* InAddress = nullptr, bool bInEntry = false)
            : Address(InAddress)
            , bEntry(bInEntry)
        {
        }

        FCode Find(std::string_view Instructions, int32_t Index = 1) const;
        FCode Last(std::string_view Instructions) const;
        FCode Call(int32_t Index = 1) const;
        FCode Start() const;

        int32_t Disp() const;
        int32_t Disp(const wchar_t* Name) const;
        int32_t Slot(const wchar_t* Name) const;
        int64_t Imm() const;
        int64_t Imm(const wchar_t* Name) const;
        const uint8_t* Target() const;
        const uint8_t* Target(const wchar_t* Name) const;

        bool Is(std::string_view Instructions) const;
        bool Has(std::string_view Instructions) const;

        const uint8_t* Get() const
        {
            return Address;
        }

        const uint8_t* Get(const wchar_t* Name) const;

    private:
        const uint8_t* Address;
        bool bEntry;
    };

    inline FCode At(const void* Address)
    {
        return FCode((const uint8_t*)Address, true);
    }

    FCode String(const wchar_t* Text);
    FCode String(const char* Text);
    FCode String(const wchar_t* Text, std::string_view Instructions);
    FCode Smallest(const wchar_t* Text);
    FCode Pattern(const char* Signature);
    FCode CallTo(const void* Function);
    FCode Reference(const void* Target, std::string_view Instructions);

    namespace Private
    {
        inline uint64_t NullFunction()
        {
            return 0;
        }

        void Log(ELogLevel Level, const wchar_t* Format, ...);
        int64_t LogSlot(const wchar_t* Name, int64_t Value);
        void** FindVTable(const wchar_t* Name);
        const uint8_t* Unhooked(const uint8_t* Target);
        void Written(const void* Address, size_t Size);
    }

    template<typename PredicateType>
    int32_t FindSlot(const void* const* VTable, PredicateType Predicate, int32_t Index = 0)
    {
        for (; IsCode(VTable[Index]); Index++)
        {
            if (Predicate(At(Private::Unhooked((const uint8_t*)VTable[Index]))))
                return Index * (int32_t)sizeof(void*);
        }

        return 0;
    }

    void* Detour(const wchar_t* Name, void* Target, void* Function);
    void* DetourCall(const wchar_t* Name, const uint8_t* Call, void* Function);
    void* DetourVTable(const wchar_t* Name, void** VTable, int64_t Slot, void* Function);
    const uint8_t* PatchBranch(const wchar_t* Name, const uint8_t* Branch, const uint8_t* Target);
    const uint8_t* Patch(const wchar_t* Name, const uint8_t* Address, std::initializer_list<uint8_t> Bytes);

    template<typename FunctionType>
    FunctionType Detour(const wchar_t* Name, const void* Target, FunctionType Function)
    {
        void* Address = nullptr;
        memcpy(&Address, &Function, sizeof(Address));

        FunctionType Original = {};
        auto Trampoline = Detour(Name, (void*)Target, Address);
        memcpy(&Original, &Trampoline, sizeof(Trampoline));

        return Original;
    }

    template<typename FunctionType>
    FunctionType DetourCall(const wchar_t* Name, const uint8_t* Call, FunctionType Function)
    {
        void* Address = nullptr;
        memcpy(&Address, &Function, sizeof(Address));

        FunctionType Original = {};
        auto Target = DetourCall(Name, Call, Address);
        memcpy(&Original, &Target, sizeof(Target));

        return Original;
    }

    template<typename FunctionType>
    FunctionType DetourVTable(const wchar_t* Name, void** VTable, int64_t Slot, FunctionType Function)
    {
        void* Address = nullptr;
        memcpy(&Address, &Function, sizeof(Address));

        FunctionType Original = {};
        auto Target = DetourVTable(Name, VTable, Slot, Address);
        memcpy(&Original, &Target, sizeof(Target));

        return Original;
    }

    template<typename ChainType>
    class TExtension
    {
    };

    template<typename ChainType>
    class TChain : public TExtension<TChain<ChainType>>
    {
    public:
        explicit TChain(const wchar_t* InName)
            : Name(InName)
        {
        }

        ChainType& At(const void* Address)
        {
            return IsActive() ? Advance(Anchor::At(Address)) : Self();
        }

        ChainType& At(const FCode& Next)
        {
            return IsActive() ? Advance(Next) : Self();
        }

        ChainType& String(const wchar_t* Text)
        {
            return IsActive() ? Advance(Anchor::String(Text)) : Self();
        }

        ChainType& String(const char* Text)
        {
            return IsActive() ? Advance(Anchor::String(Text)) : Self();
        }

        ChainType& String(const wchar_t* Text, std::string_view Instructions)
        {
            return IsActive() ? Advance(Anchor::String(Text, Instructions)) : Self();
        }

        ChainType& Smallest(const wchar_t* Text)
        {
            return IsActive() ? Advance(Anchor::Smallest(Text)) : Self();
        }

        ChainType& Pattern(const char* Signature)
        {
            return IsActive() ? Advance(Anchor::Pattern(Signature)) : Self();
        }

        ChainType& Find(std::string_view Instructions, int32_t Index = 1)
        {
            return IsActive() ? Advance(Code.Find(Instructions, Index)) : Self();
        }

        ChainType& Last(std::string_view Instructions)
        {
            return IsActive() ? Advance(Code.Last(Instructions)) : Self();
        }

        ChainType& Call(int32_t Index = 1)
        {
            return IsActive() ? Advance(Code.Call(Index)) : Self();
        }

        ChainType& Start()
        {
            return IsActive() ? Advance(Code.Start()) : Self();
        }

        ChainType& Target()
        {
            return IsActive() ? Advance(Anchor::At(Code.Target())) : Self();
        }

        ChainType& Caller()
        {
            return IsActive() ? Advance(Anchor::CallTo(Code.Start().Get()).Start()) : Self();
        }

        ChainType& Reference(std::string_view Instructions)
        {
            return IsActive() ? Advance(Anchor::Reference(Code.Get(), Instructions)) : Self();
        }

        template<typename PredicateType>
        ChainType& Virtual(const void* const* VTable, PredicateType Predicate)
        {
            if (!IsActive())
                return Self();

            auto Found = VTable ? FindSlot(VTable, Predicate) : 0;

            return IsSlot(Found) ? Enter(Found, Anchor::At(Private::Unhooked((const uint8_t*)VTable[Found / sizeof(void*)]))) : Advance(FCode());
        }

        template<typename PredicateType>
        ChainType& Virtual(const wchar_t* VTableName, PredicateType Predicate)
        {
            return IsActive() ? Virtual(Private::FindVTable(VTableName), Predicate) : Self();
        }

        template<typename... NameTypes>
        ChainType& InVirtual(const void* const* VTable, NameTypes... SlotNames)
        {
            if (!IsActive())
                return Self();

            if (!VTable)
                return Advance(FCode());

            char CallText[32];
            snprintf(CallText, sizeof(CallText), "call 0x%llx", (unsigned long long)Code.Get());

            int32_t Found = 0;

            for (const wchar_t* SlotName : { SlotNames... })
            {
                Found = FindSlot(VTable, [&](const FCode& Entry) { return Entry.Has(CallText); }, Found ? Found / (int32_t)sizeof(void*) + 1 : 0);

                if (!IsSlot(Private::LogSlot(SlotName, Found)))
                    return Advance(FCode());
            }

            return Enter(Found, Anchor::At(Private::Unhooked((const uint8_t*)VTable[Found / sizeof(void*)])).Find(CallText));
        }

        template<typename... NameTypes>
        ChainType& InVirtual(const wchar_t* VTableName, NameTypes... SlotNames)
        {
            return IsActive() ? InVirtual(Private::FindVTable(VTableName), SlotNames...) : Self();
        }

        ChainType& Or()
        {
            if (bDone || (!bFailed && Code.Get()))
            {
                bDone = true;

                return Self();
            }

            Code = FCode();
            bFailed = false;
            bVirtual = false;

            return Self();
        }

        bool IsActive() const
        {
            return !bDone && !bFailed;
        }

    protected:
        int32_t SlotValue() const
        {
            return bVirtual ? VirtualSlot : Code.Disp();
        }

        const wchar_t* Name;
        FCode Code;

    private:
        ChainType& Self()
        {
            return static_cast<ChainType&>(*this);
        }

        ChainType& Advance(const FCode& Next)
        {
            Code = Next;
            bFailed = !Code.Get();
            bVirtual = false;

            return Self();
        }

        ChainType& Enter(int32_t Found, const FCode& Next)
        {
            Advance(Next);
            VirtualSlot = Found;
            bVirtual = !bFailed;

            return Self();
        }

        int32_t VirtualSlot = 0;
        bool bVirtual = false;
        bool bFailed = false;
        bool bDone = false;
    };

    template<typename FunctionType>
    class Function : public TChain<Function<FunctionType>>
    {
    public:
        explicit Function(const wchar_t* InName)
            : TChain<Function<FunctionType>>(InName)
        {
        }

        FunctionType Get() const
        {
            auto Address = this->Code.Get(this->Name);

            return (FunctionType)(Address ? (const void*)Address : (const void*)&Private::NullFunction);
        }
    };

    template<typename FunctionType>
    class Hook : public TChain<Hook<FunctionType>>
    {
    public:
        Hook(const wchar_t* InName, FunctionType InDetour)
            : TChain<Hook<FunctionType>>(InName)
            , Detour(InDetour)
        {
        }

        FunctionType Inline() const
        {
            return Anchor::Detour(this->Name, (const void*)this->Code.Get(), Detour);
        }

        FunctionType AtCall() const
        {
            return Anchor::DetourCall(this->Name, this->Code.Get(), Detour);
        }

        FunctionType InVTable(void** VTable) const
        {
            return Anchor::DetourVTable(this->Name, VTable, VTable ? this->SlotValue() : 0, Detour);
        }

        FunctionType InVTable(const wchar_t* VTableName) const
        {
            return InVTable(Private::FindVTable(VTableName));
        }

    private:
        FunctionType Detour;
    };

    template<typename ValueType = int32_t>
    class Slot : public TChain<Slot<ValueType>>
    {
    public:
        explicit Slot(const wchar_t* InName)
            : TChain<Slot<ValueType>>(InName)
        {
        }

        ValueType Get() const
        {
            return (ValueType)Private::LogSlot(this->Name, this->SlotValue());
        }
    };
}
