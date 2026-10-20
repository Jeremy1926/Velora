#pragma once

#include <functional>
#include <string_view>
#include <type_traits>
#include "MinHook.h"
#include "Scanner.h"

extern "C"
{
#include "buffer.h"
#include "trampoline.h"
}

using Anchor::Detour;
using Anchor::DetourCall;
using Anchor::DetourVTable;
using Anchor::PatchBranch;
using Anchor::Patch;

template<typename FunctionType>
class THook : public Anchor::TChain<THook<FunctionType>>
{
public:
    THook(const wchar_t* InName, FunctionType InDetour)
        : Anchor::TChain<THook<FunctionType>>(InName)
        , Detour(InDetour)
    {
    }

    THook& If(bool bCondition)
    {
        return !this->IsActive() || bCondition ? *this : Skip();
    }

    template<typename PredicateType> requires std::is_invocable_r_v<bool, PredicateType>
    THook& If(PredicateType Predicate)
    {
        return !this->IsActive() || Predicate() ? *this : Skip();
    }

    THook& Optional()
    {
        return this->IsActive() || IsDone() ? *this : Skip();
    }

    THook& Is(std::string_view Instructions)
    {
        return IsDone() || (this->IsActive() && this->Code.Is(Instructions)) ? *this : Skip();
    }

    THook& Has(std::string_view Instructions)
    {
        return IsDone() || (this->IsActive() && this->Code.Has(Instructions)) ? *this : Skip();
    }

    THook& IsVirtual(const wchar_t* VTableName, std::string_view Instructions)
    {
        return IsDone() || (this->IsActive() && FindEntry(VTableName).Is(Instructions)) ? *this : Skip();
    }

    THook& HasVirtual(const wchar_t* VTableName, std::string_view Instructions)
    {
        return IsDone() || (this->IsActive() && FindEntry(VTableName).Has(Instructions)) ? *this : Skip();
    }

    THook& IsTarget(std::string_view Instructions)
    {
        return IsDone() || (this->IsActive() && Anchor::At(this->Code.Target()).Is(Instructions)) ? *this : Skip();
    }

    FunctionType Inline() const
    {
        return bSkipped ? FunctionType() : Anchor::Detour(this->Name, (const void*)this->Code.Get(), Detour);
    }

    FunctionType AtCall() const
    {
        return bSkipped ? FunctionType() : Anchor::DetourCall(this->Name, this->Code.Get(), Detour);
    }

    FunctionType InVTable(void** VTable) const
    {
        return bSkipped ? FunctionType() : Anchor::DetourVTable(this->Name, VTable, VTable ? this->SlotValue() : 0, Detour);
    }

    FunctionType InVTable(const wchar_t* VTableName) const
    {
        return InVTable(Scanner::FindVTable(VTableName));
    }

    FunctionType InExec(const wchar_t* FunctionPath) const
    {
        auto Function = bSkipped ? nullptr : (UFunction*)StaticFindObject(nullptr, nullptr, FunctionPath);

        if (!Function)
            return FunctionType();

        auto Exec = Function->GetNativeFunc();

        UE_LOG(LogVelora, Log, TEXT("Hooked %s, was 0x%llX"), this->Name, std::bit_cast<const uint8_t*>(Exec) - Scanner::Base());

        Function->SetNativeFunc((Native)Detour);

        return (FunctionType)Exec;
    }

private:
    bool IsDone() const
    {
        return !this->IsActive() && this->Code.Get();
    }

    THook& Skip()
    {
        bSkipped = true;

        if (!this->IsActive())
            this->Or();

        return this->At((const void*)this).Or();
    }

    Anchor::FCode FindEntry(const wchar_t* VTableName) const
    {
        auto VTable = Scanner::FindVTable(VTableName);
        auto Slot = this->SlotValue();

        return Anchor::At(VTable && Anchor::IsSlot(Slot) ? VTable[Slot / sizeof(void*)] : nullptr);
    }

    FunctionType Detour;
    bool bSkipped = false;
};

template<uint64_t Hash>
struct TDetourKey
{
};

consteval uint64_t DetourHash(std::string_view Name)
{
    uint64_t Hash = 14695981039346656037ull;

    for (auto Character : Name)
        Hash = (Hash ^ (uint8_t)Character) * 1099511628211ull;

    return Hash;
}

template<typename FunctionType, typename ObjectType>
auto DetourBind(FunctionType Function, ObjectType* Object)
{
    return [Function, Object](auto&&... Arguments) -> decltype(auto)
    {
        return std::invoke(Function, Object, std::forward<decltype(Arguments)>(Arguments)...);
    };
}

template<typename FunctionType>
const void* DetourAddress(FunctionType Function)
{
    const void* Address = nullptr;
    memcpy(&Address, &Function, sizeof(Address));

    return Address;
}

#define DETOUR_SCOPE(Class, Function, Line) DETOUR_SCOPE_LINE(Class, Function, Line)
#define DETOUR_SCOPE_LINE(Class, Function, Line) Class##_##Function##_Detour##Line

#define DETOUR(Class, Function, ...) DETOUR_IN(Class, Function, DETOUR_SCOPE(Class, Function, __LINE__), __VA_ARGS__)

#define DETOUR_IN(Class, Function, Scope, ...) \
    namespace Scope \
    { \
        namespace Anchor \
        { \
            using namespace ::Anchor; \
            \
            template<typename ClassType = Class> \
            auto Hook() \
            { \
                return THook(TEXT(#Class) TEXT("::") TEXT(#Function), &ClassType::Function); \
            } \
            \
            template<typename FunctionType> \
            auto Hook(FunctionType Detour) \
            { \
                return THook(TEXT(#Class) TEXT("::") TEXT(#Function), Detour); \
            } \
            \
            template<typename FunctionType> \
            auto Hook(const wchar_t* Name, FunctionType Detour) \
            { \
                return THook(Name, Detour); \
            } \
        } \
        \
        static const auto Original = (__VA_ARGS__); \
    } \
    \
    static auto& DetourOriginal(TDetourKey<DetourHash(#Class "::" #Function)>, decltype(&Scope::Original) = nullptr) \
    { \
        return Scope::Original; \
    }

#define Original(...) DetourBind(DetourOriginal(TDetourKey<DetourHash(__FUNCTION__)>()), this)(__VA_ARGS__)

#define DETOUR_ORIGINAL(Class, Function) DetourAddress(DetourOriginal(TDetourKey<DetourHash(#Class "::" #Function)>()))
