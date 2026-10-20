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

    THook& With(FunctionType InDetour)
    {
        if (this->IsActive())
            Detour = InDetour;

        return *this;
    }

    THook& Optional()
    {
        return this->IsActive() || this->Code.Get() ? *this : Skip();
    }

    FunctionType Inline() const
    {
        return bSkipped ? FunctionType() : Anchor::Detour(this->Name, (const void*)this->Code.Get(), Detour);
    }

    FunctionType AtCall() const
    {
        return bSkipped ? FunctionType() : Anchor::DetourCall(this->Name, this->Code.Get(), Detour);
    }

    FunctionType AtCall(const wchar_t* VTableName) const
    {
        return !bSkipped && this->Code.Is("call 0x*") ? AtCall() : InVTable(VTableName);
    }

    FunctionType AtCallsIn(const Anchor::FCode& Caller) const
    {
        FunctionType Original = {};
        auto Function = bSkipped ? nullptr : this->Code.Get();

        for (auto Call = Caller.Find("call 0x*"); Function && Call.Get(); Call = Call.Find("call 0x*"))
        {
            if (Call.Target() == Function)
                Original = Anchor::DetourCall(this->Name, Call.Get(), Detour);
        }

        return Original;
    }

    FunctionType AtReference() const
    {
        auto Reference = bSkipped ? nullptr : this->Code.Get();

        if (!Reference)
            return FunctionType();

        void* Address = nullptr;
        memcpy(&Address, &Detour, sizeof(Address));

        auto Jump = (JMP_ABS*)AllocateBuffer((LPVOID)Reference);
        *Jump = { 0xFF, 0x25, 0, (UINT64)Address };

        FunctionType Original = {};
        auto Target = this->Code.Target();
        memcpy(&Original, &Target, sizeof(Target));

        Anchor::PatchBranch(this->Name, Reference + 1, (const uint8_t*)Jump);

        return Original;
    }

    FunctionType InVTable(void** VTable) const
    {
        return bSkipped ? FunctionType() : Anchor::DetourVTable(this->Name, VTable, VTable ? this->SlotValue() : 0, Detour);
    }

    FunctionType InVTable(const wchar_t* VTableName) const
    {
        return InVTable(Scanner::FindVTable(VTableName));
    }

    template<typename SlotType> requires std::is_invocable_r_v<int32_t, SlotType>
    FunctionType InVTable(const wchar_t* VTableName, SlotType Slot) const
    {
        return bSkipped ? FunctionType() : Anchor::DetourVTable(this->Name, Scanner::FindVTable(VTableName), Slot(), Detour);
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
    THook& Skip()
    {
        bSkipped = true;

        if (!this->IsActive())
            this->Or();

        return this->At((const void*)this).Or();
    }

    FunctionType Detour;
    bool bSkipped = false;
};

class FPatch : public Anchor::TChain<FPatch>
{
public:
    explicit FPatch(const wchar_t* InName)
        : Anchor::TChain<FPatch>(InName)
    {
    }

    const uint8_t* Jump() const
    {
        auto Branch = Code.Get();

        return Branch && *Branch == 0x0F ? Anchor::Patch(Name, Branch, { 0x90, 0xE9 }) : Anchor::Patch(Name, Branch, { 0xEB });
    }

    const uint8_t* BranchTo(const void* Target) const
    {
        return Anchor::PatchBranch(Name, Code.Get(), (const uint8_t*)Target);
    }
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

#define PATCH(Class, Function, ...) PATCH_IN(Class, Function, DETOUR_SCOPE(Class, Function, __LINE__), __VA_ARGS__)

#define PATCH_IN(Class, Function, Scope, ...) \
    namespace Scope \
    { \
        namespace Anchor \
        { \
            using namespace ::Anchor; \
            \
            inline FPatch Patch() \
            { \
                return FPatch(TEXT(#Class) TEXT("::") TEXT(#Function)); \
            } \
        } \
        \
        static const auto Patched = (__VA_ARGS__); \
    }

#define Original(...) DetourBind(DetourOriginal(TDetourKey<DetourHash(__FUNCTION__)>()), this)(__VA_ARGS__)

#define ORIGINAL_OF(Class, Function) DetourBind(DetourOriginal(TDetourKey<DetourHash(#Class "::" #Function)>()), this)

#define DETOUR_ORIGINAL(Class, Function) DetourAddress(DetourOriginal(TDetourKey<DetourHash(#Class "::" #Function)>()))
