#pragma once

#include <algorithm>
#include <bit>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <Anchor/Anchor.h>
#include "Logging/LogMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

extern FLogCategoryBase LogVelora;

namespace Scanner
{
    using Anchor::Base;
    using Anchor::IsSlot;
    using Anchor::FCode;
    using Anchor::String;
    using Anchor::Reference;

    inline void Log(Anchor::ELogLevel Level, const TCHAR* Message)
    {
        if (Level == Anchor::ELogLevel::Error)
        {
            UE_LOG(LogVelora, Error, TEXT("%s"), Message);
        }
        else
        {
            UE_LOG(LogVelora, Log, TEXT("%s"), Message);
        }
    }

    inline void** FindVTable(const TCHAR* DefaultObjectPath)
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, DefaultObjectPath);

        return DefaultObject ? *(void***)DefaultObject : nullptr;
    }

    inline void** FindVTable(const TCHAR* DefaultObjectPath, const TCHAR* InterfacePath)
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, DefaultObjectPath);
        auto InterfaceClass = (UClass*)StaticFindObject(nullptr, nullptr, InterfacePath);
        auto Interface = DefaultObject && InterfaceClass ? DefaultObject->GetInterfaceAddress(InterfaceClass) : nullptr;

        return Interface ? *(void***)Interface : nullptr;
    }

    template<typename ValueType>
    inline ValueType Offset(const TCHAR* Name, ValueType Value)
    {
        if ((int64)Value > 0)
        {
            UE_LOG(LogVelora, Log, TEXT("Found %s offset 0x%llX"), Name, (int64)Value);
        }
        else
        {
            UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        }

        return Value;
    }

    inline FCode Event(const TCHAR* FunctionName)
    {
        return Reference(String(FunctionName).Find("lea rcx, [0x*]").Target(), "mov ?dx, *").Start();
    }

    inline FCode Exec(const TCHAR* FunctionPath)
    {
        auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, FunctionPath);

        return FCode(Function ? std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) : nullptr, true);
    }

    inline FCode Native(const TCHAR* FunctionPath)
    {
        auto Exec = Scanner::Exec(FunctionPath);
        std::vector<const uint8_t*> Chunks = { Exec.Get() };

        for (size_t Index = 0; Exec.Get() && Index < Chunks.size() && Index < 64; Index++)
        {
            auto Chunk = Anchor::At(Chunks[Index]);

            for (auto Pattern : { "setnz *; add [*+0x20], *", "setnz *; *; add [*+0x20], *", "setnz *; *; *; add [*+0x20], *" })
            {
                if (auto Finish = Chunk.Find(Pattern); Finish.Get())
                {
                    auto Body = Finish.Find("call 0x*");
                    auto Return = Finish.Find("ret*");

                    return Body.Get() && (!Return.Get() || Body.Get() < Return.Get()) ? Body.Call() : FCode();
                }
            }

            auto [Begin, End] = Anchor::Range(Chunks[Index]);

            for (auto Jump = Chunk.Find("j* 0x*"); Jump.Get(); Jump = Jump.Find("j* 0x*"))
            {
                auto Target = Jump.Target();

                if (Target && (Target < Begin || Target >= End) && Anchor::At(Target).Start().Get() == Exec.Get() && std::find(Chunks.begin(), Chunks.end(), Target) == Chunks.end())
                    Chunks.push_back(Target);
            }
        }

        for (auto Finish = Exec.Find("mov [*+0x20], *"); Finish.Get(); Finish = Finish.Find("mov [*+0x20], *"))
        {
            if (!Finish.Is("mov [rsp*"))
                return Finish.Call();
        }

        return FCode();
    }

    inline FCode StubExec(const TCHAR* FunctionPath)
    {
        auto Exec = Scanner::Exec(FunctionPath);

        for (auto Pattern : { "setnz *; add [*+0x20], *", "setnz *; *; add [*+0x20], *", "setnz *; *; *; add [*+0x20], *" })
        {
            auto Finish = Exec.Find(Pattern);

            if (!Finish.Get())
                continue;

            for (auto Next = Finish.Find("*"); Next.Get(); Next = Next.Find("*"))
            {
                if (Next.Is("ret*"))
                    return Finish;

                if ((Next.Is("jmp 0x*") && Anchor::FCode(Next.Target()).Start().Get() != Exec.Get()) || Next.Is("jmp [*") || Next.Is("call [*") || (Next.Is("call 0x*") && !Anchor::At(Next.Target()).Is("cmp rcx, [0x*]")))
                    return FCode();

                if ((Next.Is("mov* [*], *") || Next.Is("mov* * ptr [*], *") || Next.Is("and * ptr [*], *") || Next.Is("or * ptr [*], *")) && !Next.Is("mov* [rsp*") && !Next.Is("mov* * ptr [rsp*") && !Next.Is("mov * ptr [*], 0x0") && !Next.Is("and * ptr [*], 0x0"))
                    return FCode();
            }
        }

        return FCode();
    }

    inline uint64 NullFunction()
    {
        return 0;
    }

    template<typename FunctionType>
    inline FunctionType Function(const void* Address)
    {
        return (FunctionType)(Address ? Address : (const void*)&NullFunction);
    }

    template<typename ReturnType, typename... ArgTypes>
    inline auto NullVirtual(ReturnType (*)(ArgTypes...))
    {
        return +[](ArgTypes...) -> ReturnType
        {
            return ReturnType();
        };
    }

    template<typename ChainType>
    struct TChainAccess : Anchor::TChain<ChainType>
    {
        using Anchor::TChain<ChainType>::Code;
        using Anchor::TChain<ChainType>::SlotValue;
    };

    template<typename FunctionType>
    inline FunctionType Virtual(const void* Object, int64 Slot, int64 Offset = 0)
    {
        if (IsSlot(Slot) && IsSlot(Slot + Offset))
            return (*(FunctionType* const*)Object)[(Slot + Offset) / sizeof(void*)];

        if constexpr (std::is_function_v<std::remove_pointer_t<FunctionType>>)
            return NullVirtual(FunctionType());
        else
            return nullptr;
    }
}

namespace Anchor
{
    template<typename ChainType>
    class TExtension<TChain<ChainType>>
    {
    public:
        ChainType& Exec(const TCHAR* FunctionPath)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Scanner::Exec(FunctionPath)) : Chain;
        }

        ChainType& Exec(const UFunction* Function)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(FCode(Function ? std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) : nullptr, true)) : Chain;
        }

        ChainType& Native(const TCHAR* FunctionPath)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Scanner::Native(FunctionPath)) : Chain;
        }

        ChainType& StubExec(const TCHAR* FunctionPath)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Scanner::StubExec(FunctionPath)) : Chain;
        }

        ChainType& Event(const TCHAR* FunctionName)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Scanner::Event(FunctionName)) : Chain;
        }

        ChainType& Body(const TCHAR* FunctionPath)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive())
                return Chain;

            auto Finish = Scanner::Exec(FunctionPath).Find("setnz *; add [*+0x20], *; jmp 0x*");

            if (!Finish.Get())
                Finish = Scanner::Exec(FunctionPath).Find("setnz *; *; add [*+0x20], *; jmp 0x*");

            if (!Finish.Get())
                Finish = Scanner::Exec(FunctionPath).Find("mov [*+0x20], *; jmp 0x*");

            return Chain.At(Finish.Get() ? Anchor::At(Finish.Find("jmp 0x*").Target()) : Scanner::Native(FunctionPath));
        }

        ChainType& Log(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At((Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get(Name)) : Chain;
        }

        ChainType& FindCall(const void* Function, int32_t Index = 1)
        {
            return FindTo("call", Function, Index);
        }

        ChainType& FindCallToString(const TCHAR* Text, int32_t Index = 1)
        {
            return FindCall(Anchor::String(Text).Start().Get(), Index);
        }

        ChainType& CallIn(const void* Caller)
        {
            return CallIn(Anchor::At(Caller));
        }

        ChainType& FirstCalleeCalling(const void* Function)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            char Call[48];
            snprintf(Call, sizeof(Call), "call 0x%llx", (unsigned long long)Function);
            auto Callee = FCode();

            for (auto Found = (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Find("call 0x*"); Chain.IsActive() && Found.Get(); Found = Found.Find("call 0x*"))
            {
                if (Found.Target() != Function && Anchor::At(Found.Target()).Has(Call))
                {
                    Callee = Anchor::At(Found.Target());
                    break;
                }
            }

            return Chain.IsActive() ? Chain.At(Callee) : Chain;
        }

        ChainType& CallIn(const FCode& Caller)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Function = (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get();

            return Chain.IsActive() ? Chain.At(Caller).FindCall(Function) : Chain;
        }

        ChainType& CallSite()
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Anchor::CallTo((Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get())) : Chain;
        }

        ChainType& CallUntil(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));

            while (Code.Get() && !Code.Has(Instructions))
                Code = Code.Call();

            return Chain.IsActive() ? Chain.At(Code) : Chain;
        }

        ChainType& FindEither(std::string_view First, std::string_view Second)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto Found = Code.Find(First);

            return Chain.IsActive() ? Chain.At(Found.Get() ? Found : Code.Find(Second)) : Chain;
        }

        ChainType& AdjacentOrLast(std::string_view Adjacent, std::string_view Previous)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto Next = Code.Find("*");

            return Chain.IsActive() ? Chain.At(Next.Is(Adjacent) ? Next : Code.Last(Previous)) : Chain;
        }

        ChainType& TryCall(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto Callee = Code.Find(Instructions).Call();

            return Chain.IsActive() ? Chain.At(Callee.Get() ? Callee : Code) : Chain;
        }

        ChainType& LastRepeatedCall()
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Call = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));

            while (Call.Get() && Call.Last("call 0x*").Target() != Call.Target())
                Call = Call.Last("call 0x*");

            return Chain.IsActive() ? Chain.At(Call) : Chain;
        }

        ChainType& FirstOf(std::string_view First, std::string_view Second)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto FirstFound = Code.Find(First);
            auto SecondFound = Code.Find(Second);

            return Chain.IsActive() ? Chain.At(FirstFound.Get() && (!SecondFound.Get() || FirstFound.Get() < SecondFound.Get()) ? FirstFound : SecondFound) : Chain;
        }

        ChainType& ForwarderIn(const TCHAR* VTableName)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            char Forward[64];
            snprintf(Forward, sizeof(Forward), "mov rax, [rcx]; jmp [rax+0x%llx]", (unsigned long long)(Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))());

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Is(Forward); }) : Chain;
        }

        ChainType& FirstTo(const char* Mnemonic)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto Target = Code.Target();

            return Chain.IsActive() ? Chain.At(Code.Start()).FindTo(Mnemonic, Target) : Chain;
        }

        ChainType& VirtualCaller(const TCHAR* VTableName, std::string_view Before = "")
        {
            auto& Chain = static_cast<ChainType&>(*this);
            char Call[48];
            snprintf(Call, sizeof(Call), "call 0x%llx", (unsigned long long)(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get());
            auto Text = std::string(Before) + Call;

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Has(Text); }) : Chain;
        }

        template<typename PropertyType>
        ChainType& BoolGetter(const TCHAR* VTableName, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            auto Offset = Property->GetOffset_ForInternal() + Property->GetByteOffset();
            auto Bit = std::countr_zero(Property->GetFieldMask());
            char Getter[96];

            if (Bit)
                snprintf(Getter, sizeof(Getter), "mov* *, *[rcx+0x%x]; shr al, 0x%x; and *", (uint32_t)Offset, (uint32_t)Bit);
            else
                snprintf(Getter, sizeof(Getter), "mov* *, *[rcx+0x%x]; and *", (uint32_t)Offset);

            return Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Is(Getter); });
        }

        template<typename PropertyType>
        ChainType& FieldUserIn(const TCHAR* VTableName, std::string_view Instructions, const PropertyType* Property, std::string_view AlsoHas = "")
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Text(Instructions);
            char Offset[16];
            snprintf(Offset, sizeof(Offset), "0x%x", (uint32_t)Property->GetOffset_ForInternal());
            Text.replace(Text.find("%x"), 2, Offset);

            return Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Start().Has(Text) && (AlsoHas.empty() || Entry.Start().Has(AlsoHas)); });
        }

        ChainType& Strings(const TCHAR* First, const TCHAR* Second)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Found = Anchor::String(First);

            return Chain.IsActive() ? Chain.At(Found.Get() ? Found : Anchor::String(Second)) : Chain;
        }

        ChainType& SameCall(int32_t First, int32_t Second)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));

            return Chain.IsActive() && Code.Call(First).Get() != Code.Call(Second).Get() ? Chain.At(FCode()) : Chain;
        }

        ChainType& FindSlotCall(int64_t Slot)
        {
            char Call[48];
            snprintf(Call, sizeof(Call), "call [*+0x%llx]", (unsigned long long)Slot);

            return static_cast<ChainType&>(*this).Find(Call);
        }

        template<typename PropertyType>
        ChainType& FieldHas(std::string_view Instructions, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Text(Instructions);
            char Offset[16];
            snprintf(Offset, sizeof(Offset), "0x%x", (uint32_t)Property->GetOffset_ForInternal());
            Text.replace(Text.find("%x"), 2, Offset);

            return Has(Text);
        }

        ChainType& VirtualIs(const TCHAR* VTableName, std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            std::string Text(Instructions);
            char Target[32];
            snprintf(Target, sizeof(Target), "0x%llx", (unsigned long long)(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get());
            Text.replace(Text.find("%t"), 2, Target);

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Is(Text); }) : Chain;
        }

        ChainType& VirtualSlotCaller(const TCHAR* VTableName, std::string_view Before = "")
        {
            auto& Chain = static_cast<ChainType&>(*this);
            char Call[48];
            snprintf(Call, sizeof(Call), "call [rax+0x%llx]", (unsigned long long)(Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))());
            auto Text = std::string(Before) + Call;

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Start().Has(Text); }) : Chain;
        }

        template<typename PropertyType>
        ChainType& FieldReaderPair(const TCHAR* VTableName, const PropertyType* First, const PropertyType* Second)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto VTable = (const uint8_t* const*)Scanner::FindVTable(VTableName);
            auto Image = Anchor::Base();
            auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;

            auto Reads = [&](int32_t Index, const PropertyType* Property)
            {
                char Add[32];
                char Lea[48];
                snprintf(Add, sizeof(Add), "add rcx, 0x%x", (uint32_t)Property->GetOffset_ForInternal());
                snprintf(Lea, sizeof(Lea), "lea rcx, [*+0x%x]", (uint32_t)Property->GetOffset_ForInternal());

                auto Entry = Anchor::Private::Unhooked(VTable[Index]);

                return Entry >= Image && Entry < ImageEnd && (Anchor::At(Entry).Has(Add) || Anchor::At(Entry).Has(Lea));
            };

            int64_t Found = 0;

            for (int32_t Index = 0; Chain.IsActive() && VTable && First && Second && Anchor::IsCode(VTable[Index + 1]); Index++)
            {
                if (Reads(Index, First) && !Reads(Index, Second) && Reads(Index + 1, Second) && !Reads(Index + 1, First))
                {
                    Found = Index * sizeof(void*);
                    break;
                }
            }

            return Chain.IsActive() ? EnterSlot((const void* const*)VTable, Found) : Chain;
        }

        ChainType& IsCode()
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !Anchor::IsCode((Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get()) ? Chain.At(FCode()) : Chain;
        }

        template<typename PropertyType>
        ChainType& BoolTestIn(const void* Object, const PropertyType* Property, std::string_view After)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto VTable = Object ? *(const uint8_t* const* const*)Object : nullptr;

            if (!Chain.IsActive() || !VTable || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            auto Image = Anchor::Base();
            auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;
            auto Offset = Property->GetOffset_ForInternal() + Property->GetByteOffset();
            char Check[64];

            if (Property->GetFieldMask() == 0xFF)
                snprintf(Check, sizeof(Check), "cmp byte ptr [rcx+0x%x], 0x0", (uint32_t)Offset);
            else
                snprintf(Check, sizeof(Check), "test byte ptr [rcx+0x%x], 0x%x", (uint32_t)Offset, (uint32_t)Property->GetFieldMask());

            auto Text = std::string(Check) + std::string(After);
            int64_t Found = 0;

            for (int32_t Index = 0; Anchor::IsCode(VTable[Index]); Index++)
            {
                if (auto Entry = Anchor::Private::Unhooked(VTable[Index]); Entry >= Image && Entry < ImageEnd && Anchor::At(Entry).Has(Text))
                {
                    Found = Index * sizeof(void*);
                    break;
                }
            }

            return EnterSlot((const void* const*)VTable, Found);
        }

        ChainType& SlotHere(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto VTable = (const void* const*)(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get();

            return Chain.IsActive() ? Chain.Virtual(VTable, [&](const FCode& Entry) { return Entry.Is(Instructions); }) : Chain;
        }

        ChainType& UseSlot(const TCHAR* VTableName, int64_t Slot)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? EnterSlot(Scanner::FindVTable(VTableName), Slot) : Chain;
        }

        template<typename PropertyType>
        ChainType& FieldUserOf(const void* Object, std::string_view Instructions, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Object || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Text(Instructions);
            char Offset[16];
            snprintf(Offset, sizeof(Offset), "0x%x", (uint32_t)Property->GetOffset_ForInternal());
            Text.replace(Text.find("%x"), 2, Offset);

            auto Image = Anchor::Base();
            auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;

            return Chain.Virtual(*(const void* const* const*)Object, [&](const FCode& Entry) { return Entry.Get() >= Image && Entry.Get() < ImageEnd && Entry.Has(Text); });
        }

        ChainType& NameUserOf(const void* Object, const char* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Object)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            auto Image = Anchor::Base();
            auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;
            auto Size = strlen(Name) + 1;

            return Chain.Virtual(*(const void* const* const*)Object, [&](const FCode& Entry)
            {
                if (Entry.Get() < Image || Entry.Get() >= ImageEnd)
                    return false;

                for (auto Load = Entry.Find("lea rdx, [0x*]"); Load.Get(); Load = Load.Find("lea rdx, [0x*]"))
                {
                    auto Target = Load.Target();

                    if (Target >= Image && Target + Size <= ImageEnd && !memcmp(Target, Name, Size))
                        return true;
                }

                return false;
            });
        }

        template<typename PropertyType>
        ChainType& FieldUser(const TCHAR* VTableName, std::string_view Instructions, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Text(Instructions);
            char Offset[16];
            snprintf(Offset, sizeof(Offset), "0x%x", (uint32_t)Property->GetOffset_ForInternal());
            Text.replace(Text.find("%x"), 2, Offset);

            return Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Has(Text); });
        }

        template<typename... PatternTypes>
        ChainType& VirtualMatch(const TCHAR* VTableName, PatternTypes... Patterns)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return (Entry.Is(Patterns) || ...); }) : Chain;
        }

        ChainType& VirtualStartHas(const void* const* VTable, std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !VTable)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            return Chain.Virtual(VTable, [&](const FCode& Entry) { return Entry.Start().Has(Instructions); });
        }

        ChainType& VirtualLoading(const TCHAR* VTableName)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            char Load[48];
            snprintf(Load, sizeof(Load), "lea *, [0x%llx]", (unsigned long long)(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Target());

            return Chain.IsActive() ? VirtualStartHas(Scanner::FindVTable(VTableName), Load) : Chain;
        }

        ChainType& SlotOrLastImm(const void* Object, std::string_view Previous)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));

            if (!Chain.IsActive() || Code.Disp())
                return Chain;

            return EnterSlot(Object ? *(const void* const* const*)Object : nullptr, (int32_t)Code.Last(Previous).Imm());
        }

        ChainType& FindTo(const char* Mnemonic, const void* Target, int32_t Index = 1)
        {
            char Branch[48];
            snprintf(Branch, sizeof(Branch), "%s 0x%llx", Mnemonic, (unsigned long long)Target);

            return static_cast<ChainType&>(*this).Find(Branch, Index);
        }

        template<typename PropertyType>
        ChainType& FieldPattern(std::string_view Signature, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Pattern;

            for (size_t Index = 0; Index < Signature.size(); Index++)
            {
                if (Signature[Index] != '%' || Index + 1 == Signature.size())
                {
                    Pattern += Signature[Index];
                    continue;
                }

                auto Token = Signature[++Index];
                uint32_t Value = Property->GetOffset_ForInternal();
                auto Count = 4;

                if constexpr (requires { Property->GetFieldMask(); })
                {
                    if (Token == 'b')
                        Value += Property->GetByteOffset();

                    if (Token == 'm')
                    {
                        Value = Property->GetFieldMask();
                        Count = 1;
                    }
                }

                for (auto Byte = 0; Byte < Count; Byte++)
                {
                    char Text[4];
                    snprintf(Text, sizeof(Text), Byte ? " %02X" : "%02X", (Value >> (Byte * 8)) & 0xFF);
                    Pattern += Text;
                }
            }

            return Chain.Pattern(Pattern.c_str());
        }

        template<typename PropertyType>
        ChainType& FieldFind(std::string_view Instructions, const PropertyType* Property)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            std::string Text(Instructions);
            auto Token = Text.find("%x");
            char Offset[16];
            snprintf(Offset, sizeof(Offset), "0x%x", (uint32_t)Property->GetOffset_ForInternal());

            if (Token != std::string::npos)
                Text.replace(Token, 2, Offset);

            return Chain.Find(Text);
        }

        ChainType& LastCall(const void* Function)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            FCode Found;

            for (auto Call = (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Find("call 0x*"); Chain.IsActive() && Function && Call.Get(); Call = Call.Find("call 0x*"))
            {
                if (Call.Target() == Function)
                    Found = Call;
            }

            return Chain.IsActive() ? Chain.At(Found) : Chain;
        }

        ChainType& If(bool bCondition)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !bCondition ? Chain.At(FCode()) : Chain;
        }

        ChainType& Is(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Is(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& Has(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Has(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& IsVirtual(const void* Object, std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !EntryOf(Object ? *(const void* const* const*)Object : nullptr).Is(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& IsVirtual(const TCHAR* VTableName, std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !EntryOf(Scanner::FindVTable(VTableName)).Is(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& HasVirtual(const TCHAR* VTableName, std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !EntryOf(Scanner::FindVTable(VTableName)).Has(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& IsTarget(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && !Anchor::At((Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Target()).Is(Instructions) ? Chain.At(FCode()) : Chain;
        }

        ChainType& IsCallTo(const void* Function)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() && (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Target() != Function ? Chain.At(FCode()) : Chain;
        }

        ChainType& LogSlot(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (Chain.IsActive())
                Private::LogSlot(Name, (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))());

            return Chain;
        }

        ChainType& LogSlot(const TCHAR* Name, int64_t Offset)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Slot = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))();

            if (Chain.IsActive() && IsSlot(Slot))
                Private::LogSlot(Name, Slot + Offset);

            return Chain;
        }

        ChainType& Optional()
        {
            auto& Chain = static_cast<ChainType&>(*this);

            bMissing = !Chain.IsActive() && !(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get();

            return Chain;
        }

        ChainType& Origin(std::string_view Instructions)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (Chain.IsActive())
                OriginValue = (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Find(Instructions).Disp();

            return Chain;
        }

        ChainType& LastSince(std::string_view Instructions, std::string_view Barrier)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = (Chain.*(&Scanner::TChainAccess<ChainType>::Code));
            auto Found = Code.Last(Instructions);

            return Chain.IsActive() ? Chain.At(Found.Get() > Code.Last(Barrier).Get() ? Found : FCode()) : Chain;
        }

        template<typename... PropertyTypes>
        ChainType& FieldVirtual(const TCHAR* VTableName, std::string_view Instructions, const PropertyTypes*... Properties)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            std::string Text(Instructions);

            for (auto Offset : { (uint32_t)Properties->GetOffset_ForInternal()... })
            {
                char Value[16];
                snprintf(Value, sizeof(Value), "0x%x", Offset);
                Text.replace(Text.find("%x"), 2, Value);
            }

            return Chain.IsActive() ? Chain.Virtual(VTableName, [&](const FCode& Entry) { return Entry.Is(Text); }) : Chain;
        }

        template<typename PropertyType, typename... PatternTypes>
        ChainType& FieldVirtualAny(const TCHAR* VTableName, const PropertyType* Property, PatternTypes... Patterns)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (!Chain.IsActive() || !Property)
                return Chain.IsActive() ? Chain.At(FCode()) : Chain;

            char Value[16];
            snprintf(Value, sizeof(Value), "0x%x", (uint32_t)Property->GetOffset_ForInternal());

            std::string Texts[] = { std::string(Patterns)... };

            for (auto& Text : Texts)
                Text.replace(Text.find("%x"), 2, Value);

            return Chain.Virtual(VTableName, [&](const FCode& Entry)
            {
                for (auto& Text : Texts)
                {
                    if (Entry.Is(Text))
                        return true;
                }

                return false;
            });
        }

        ChainType& SlotIn(const TCHAR* VTableName)
        {
            return SlotOf(Scanner::FindVTable(VTableName));
        }

        ChainType& SlotIn(const void* Object)
        {
            return SlotOf(Object ? *(const void* const* const*)Object : nullptr);
        }

        ChainType& Entry(const TCHAR* VTableName)
        {
            return EntryIn(Scanner::FindVTable(VTableName));
        }

        ChainType& Entry(const void* Object)
        {
            return EntryIn(Object ? *(const void* const* const*)Object : nullptr);
        }

        ChainType& Entry(const TCHAR* VTableName, const TCHAR* InterfacePath)
        {
            return EntryIn(Scanner::FindVTable(VTableName, InterfacePath));
        }

        ChainType& SlotShift(const TCHAR* VTableName, int64_t Bytes)
        {
            return SlotShiftIn(Scanner::FindVTable(VTableName), Bytes);
        }

        ChainType& SlotShift(const void* Object, int64_t Bytes)
        {
            return SlotShiftIn(Object ? *(const void* const* const*)Object : nullptr, Bytes);
        }

        ChainType& SlotForwarded(const TCHAR* VTableName)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Slot = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))();
            char Next[32];
            snprintf(Next, sizeof(Next), "jmp [*+0x%llx]", (unsigned long long)(Slot + sizeof(void*)));

            return Chain.IsActive() && EntryOf(Scanner::FindVTable(VTableName)).Has(Next) ? EnterSlot(Scanner::FindVTable(VTableName), Slot + sizeof(void*)) : Chain;
        }

        ChainType& SelfCallerBelow(const TCHAR* VTableName)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto VTable = Scanner::FindVTable(VTableName);
            int64_t Found = 0;

            for (auto Index = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))() / (int64_t)sizeof(void*) - 1; Chain.IsActive() && VTable && Index > 0; Index--)
            {
                char Call[32];
                char Jump[32];
                char Load[48];
                snprintf(Call, sizeof(Call), "call [*+0x%llx]", (unsigned long long)(Index * sizeof(void*)));
                snprintf(Jump, sizeof(Jump), "jmp [*+0x%llx]", (unsigned long long)(Index * sizeof(void*)));
                snprintf(Load, sizeof(Load), "mov r10, [*+0x%llx]; jmp r10", (unsigned long long)(Index * sizeof(void*)));

                if (auto Entry = Anchor::At(Anchor::Private::Unhooked((const uint8_t*)VTable[Index])); Entry.Has(Call) || Entry.Has(Jump) || Entry.Has(Load))
                {
                    Found = Index * sizeof(void*);
                    break;
                }
            }

            return Chain.IsActive() ? EnterSlot(VTable, Found) : Chain;
        }

    protected:
        bool bMissing = false;
        int64_t OriginValue = 0;

    private:
        ChainType& SlotShiftIn(const void* const* VTable, int64_t Bytes)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Slot = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))();

            return Chain.IsActive() ? EnterSlot(VTable, IsSlot(Slot) ? Slot + Bytes : 0) : Chain;
        }

        ChainType& EnterSlot(const void* const* VTable, int64_t Slot)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.Virtual(VTable, [Index = (int64_t)0, Slot](const FCode&) mutable { return Index++ * (int64_t)sizeof(void*) == Slot; });
        }

        ChainType& SlotOf(const void* const* VTable)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto Address = (Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get();

            return Chain.Virtual(VTable, [Address](const FCode& Entry) { return Entry.Get() == Address; });
        }

        ChainType& EntryIn(const void* const* VTable)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(EntryOf(VTable)) : Chain;
        }

        FCode EntryOf(const void* const* VTable)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto VTableSlot = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))();

            return Anchor::At(VTable && IsSlot(VTableSlot) ? Anchor::Private::Unhooked((const uint8_t*)VTable[VTableSlot / sizeof(void*)]) : nullptr);
        }
    };

    template<typename FunctionType>
    struct TVirtualCall
    {
        int64_t Slot = 0;

        explicit operator bool() const
        {
            return IsSlot(Slot);
        }

        template<typename OtherFunctionType>
        TVirtualCall<OtherFunctionType> As() const
        {
            return { Slot };
        }

        template<typename ObjectType, typename... ArgumentTypes>
        decltype(auto) operator()(ObjectType* Object, ArgumentTypes&&... Arguments) const
        {
            return Scanner::Virtual<FunctionType>(Object, Slot)(Object, std::forward<ArgumentTypes>(Arguments)...);
        }
    };

    template<typename FunctionType>
    class VirtualFunction : public TChain<VirtualFunction<FunctionType>>
    {
    public:
        explicit VirtualFunction(const wchar_t* InName)
            : TChain<VirtualFunction<FunctionType>>(InName)
        {
        }

        TVirtualCall<FunctionType> Get(int64_t Offset = 0) const
        {
            if (this->bMissing)
                return {};

            auto Slot = Private::LogSlot(this->Name, this->SlotValue());

            return { IsSlot(Slot) && IsSlot(Slot + Offset) ? Slot + Offset : 0 };
        }

        FunctionType Get(const wchar_t* DefaultObjectPath) const
        {
            return Scanner::Virtual<FunctionType>(StaticFindObject(nullptr, nullptr, DefaultObjectPath), Private::LogSlot(this->Name, this->SlotValue()));
        }
    };

    template<typename PointerType>
    class Data : public TChain<Data<PointerType>>
    {
    public:
        explicit Data(const wchar_t* InName)
            : TChain<Data<PointerType>>(InName)
        {
        }

        PointerType Get() const
        {
            return this->bMissing ? nullptr : (PointerType)this->Code.Get(this->Name);
        }
    };

    template<typename PointerType>
    class Optional : public TChain<Optional<PointerType>>
    {
    public:
        explicit Optional(const wchar_t* InName)
            : TChain<Optional<PointerType>>(InName)
        {
        }

        PointerType Get() const
        {
            return this->Code.Get() ? (PointerType)this->Code.Get(this->Name) : nullptr;
        }
    };

    class Address : public TChain<Address>
    {
    public:
        Address()
            : TChain<Address>(nullptr)
        {
        }

        const uint8_t* Get() const
        {
            return this->Code.Get();
        }
    };

    class Exists : public TChain<Exists>
    {
    public:
        Exists()
            : TChain<Exists>(nullptr)
        {
        }

        bool Get() const
        {
            return this->Code.Get() != nullptr;
        }
    };

    template<typename FunctionType>
    class Unnamed : public TChain<Unnamed<FunctionType>>
    {
    public:
        Unnamed()
            : TChain<Unnamed<FunctionType>>(nullptr)
        {
        }

        FunctionType Get() const
        {
            return Scanner::Function<FunctionType>(this->Code.Get());
        }
    };

    class Offset : public TChain<Offset>
    {
    public:
        explicit Offset(const wchar_t* InName)
            : TChain<Offset>(InName)
        {
        }

        int32_t Disp() const
        {
            return this->bMissing ? 0 : (int32_t)Scanner::Offset(this->Name, (int64_t)this->Code.Disp() - this->OriginValue);
        }

        int64_t Imm() const
        {
            return this->bMissing ? 0 : this->Code.Imm(this->Name);
        }

        int64_t Get() const
        {
            return this->bMissing ? 0 : Scanner::Offset(this->Name, this->Code.Disp() ? (int64_t)this->Code.Disp() - this->OriginValue : this->Code.Imm());
        }

        int64_t Sum() const
        {
            return this->bMissing ? 0 : Scanner::Offset(this->Name, this->Code.Imm() + this->Code.Disp());
        }

        int64_t Value() const
        {
            return this->Code.Imm();
        }

        int32_t DispValue() const
        {
            return this->Code.Disp();
        }

        int64_t Mask() const
        {
            if (this->Code.Is("bts *"))
                return 1ll << this->Code.Imm();

            return this->Code.Is("and *") ? ~this->Code.Imm() : this->Code.Imm();
        }
    };
}
