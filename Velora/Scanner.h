#pragma once

#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <Anchor/Anchor.h>
#include "Logging/LogMacros.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

extern FLogCategoryBase LogVelora;

namespace Scanner
{
    using Anchor::Base;
    using Anchor::IsCode;
    using Anchor::IsSlot;
    using Anchor::FCode;
    using Anchor::At;
    using Anchor::String;
    using Anchor::Smallest;
    using Anchor::Pattern;
    using Anchor::CallTo;
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

    template<typename ValueType>
    inline ValueType Slot(const TCHAR* Name, ValueType Value)
    {
        if (IsSlot((int64)Value))
        {
            UE_LOG(LogVelora, Log, TEXT("Found %s vtable slot 0x%llX"), Name, (int64)Value);
        }
        else
        {
            UE_LOG(LogVelora, Error, TEXT("Could not find %s"), Name);
        }

        return Value;
    }

    template<typename PredicateType>
    inline int32 Slot(const TCHAR* Name, const void* const* VTable, PredicateType Predicate, int32 Index = 0)
    {
        return Slot(Name, Anchor::FindSlot(VTable, Predicate, Index));
    }

    inline FCode Event(const TCHAR* FunctionName)
    {
        return Reference(String(FunctionName).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start();
    }

    inline FCode Exec(const TCHAR* FunctionPath)
    {
        auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, FunctionPath);

        return FCode(Function ? std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) : nullptr, true);
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

        ChainType& Native(const TCHAR* FunctionPath)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At(Scanner::Native(FunctionPath)) : Chain;
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

            auto Finish = Scanner::Exec(FunctionPath).Find("mov [*+0x20], *; jmp 0x*");

            return Chain.At(Finish.Get() ? Anchor::At(Finish.Find("jmp 0x*").Target()) : Scanner::Native(FunctionPath));
        }

        ChainType& Log(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            return Chain.IsActive() ? Chain.At((Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get(Name)) : Chain;
        }

        ChainType& FindCall(const void* Function, int32_t Index = 1)
        {
            char Call[32];
            snprintf(Call, sizeof(Call), "call 0x%llx", (unsigned long long)Function);

            return static_cast<ChainType&>(*this).Find(Call, Index);
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

        ChainType& LogSlot(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (Chain.IsActive())
                Private::LogSlot(Name, (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))());

            return Chain;
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

    private:
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

            return Anchor::At(VTable && IsSlot(VTableSlot) ? VTable[VTableSlot / sizeof(void*)] : nullptr);
        }
    };
}
