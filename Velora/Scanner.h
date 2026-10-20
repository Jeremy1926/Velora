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

        ChainType& Log(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);
            auto& Code = Chain.*(&Scanner::TChainAccess<ChainType>::Code);

            if (!Chain.IsActive() && Code.Get())
                return Chain;

            auto Address = Code.Get(Name);

            return Chain.IsActive() ? Chain.At(Address) : Chain;
        }

        ChainType& FindCall(const void* Function, int32_t Index = 1)
        {
            char Call[32];
            snprintf(Call, sizeof(Call), "call 0x%llx", (unsigned long long)Function);

            return static_cast<ChainType&>(*this).Find(Call, Index);
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

        ChainType& LogSlot(const TCHAR* Name)
        {
            auto& Chain = static_cast<ChainType&>(*this);

            if (Chain.IsActive() || !(Chain.*(&Scanner::TChainAccess<ChainType>::Code)).Get())
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
            auto VTableSlot = (Chain.*(&Scanner::TChainAccess<ChainType>::SlotValue))();

            return Chain.IsActive() ? Chain.At(VTable && IsSlot(VTableSlot) ? VTable[VTableSlot / sizeof(void*)] : nullptr) : Chain;
        }
    };
}
