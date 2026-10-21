#pragma once

#include <functional>
#include <string_view>
#include <type_traits>
#include "MinHook.h"
#include "Scanner.h"
#include "UObject/UObjectArray.h"

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

    THook& After(const void*)
    {
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

    FunctionType AtCallPair() const
    {
        if (bSkipped)
            return FunctionType();

        Anchor::DetourCall(this->Name, this->Code.Last("call 0x*").Get(), Detour);

        return Anchor::DetourCall(this->Name, this->Code.Get(), Detour);
    }

    FunctionType AtCall(const wchar_t* VTableName) const
    {
        return !bSkipped && this->Code.Is("call 0x*") ? AtCall() : InVTable(VTableName);
    }

    FunctionType AtCallsTo(const void* Function) const
    {
        FunctionType Original = {};
        auto Caller = bSkipped ? Anchor::FCode() : this->Code;

        for (auto Call = Caller.Find("call 0x*"); Function && Call.Get(); Call = Call.Find("call 0x*"))
        {
            if (Call.Target() == Function)
                Original = Anchor::DetourCall(this->Name, Call.Get(), Detour);
        }

        return Original;
    }

    FunctionType AtCallsToInCallees(const void* Function) const
    {
        FunctionType Original = {};
        auto Caller = bSkipped ? Anchor::FCode() : this->Code;
        char Text[48];
        snprintf(Text, sizeof(Text), "call 0x%llx", (unsigned long long)Function);

        for (auto Call = Caller.Find("call 0x*"); Function && Call.Get(); Call = Call.Find("call 0x*"))
        {
            if (Call.Target() == Function || !Anchor::At(Call.Target()).Has(Text))
                continue;

            for (auto Inner = Anchor::At(Call.Target()).Find("call 0x*"); Inner.Get(); Inner = Inner.Find("call 0x*"))
            {
                if (Inner.Target() == Function)
                    Original = Anchor::DetourCall(this->Name, Inner.Get(), Detour);
            }
        }

        return Original;
    }

    FunctionType AtCallsIn(const uint8_t* Caller) const
    {
        return AtCallsIn(Anchor::At(Caller));
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

    FunctionType InInterfaceVTable(const wchar_t* DefaultObjectPath, const wchar_t* InterfacePath) const
    {
        return InVTable(Scanner::FindVTable(DefaultObjectPath, InterfacePath));
    }

    FunctionType InChildVTables(UClass* OwnerClass) const
    {
        auto Slot = this->SlotValue();
        auto Stub = bSkipped || !OwnerClass ? nullptr : Scanner::Virtual<const void*>(OwnerClass->GetDefaultObject(false), Slot);
        FunctionType Original = {};

        for (int32 Index = 0; Stub && Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(OwnerClass))
                continue;

            auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false);

            if (DefaultObject && Scanner::Virtual<const void*>(DefaultObject, Slot) == Stub)
                Original = Anchor::DetourVTable(this->Name, *(void***)DefaultObject, Slot, Detour);
        }

        return Original;
    }

    FunctionType InChildVTablesCounted(UClass* OwnerClass) const
    {
        auto Slot = this->SlotValue();
        auto Stub = bSkipped || !OwnerClass ? nullptr : Scanner::Virtual<const void*>(OwnerClass->GetDefaultObject(false), Slot);
        FunctionType Original = {};
        auto Classes = 0;

        for (int32 Index = 0; !bSkipped && Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(OwnerClass))
                continue;

            auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false);

            if (DefaultObject && Stub && Scanner::Virtual<const void*>(DefaultObject, Slot) == Stub)
            {
                Original = Anchor::DetourVTable(nullptr, *(void***)DefaultObject, Slot, Detour);
                Classes++;
            }
        }

        if (!bSkipped)
            UE_LOG(LogVelora, Log, TEXT("Hooked %s in %d classes"), this->Name, Classes);

        return Original;
    }

    FunctionType InInterfaceVTables(UClass* BaseClass, UClass* Interface, std::string_view Instructions) const
    {
        auto Slot = this->SlotValue();
        FunctionType Original = {};
        auto Classes = 0;

        for (int32 Index = 0; !bSkipped && Index < GUObjectArray.GetObjectArrayNum(); Index++)
        {
            auto ObjectItem = GUObjectArray.IndexToObject(Index);
            auto Object = ObjectItem ? (UObject*)ObjectItem->Object : nullptr;

            if (!Object || !Object->IsA(UClass::StaticClass()) || !((UClass*)Object)->IsChildOf(BaseClass))
                continue;

            auto DefaultObject = ((UClass*)Object)->GetDefaultObject(false);
            auto InterfaceObject = DefaultObject ? DefaultObject->GetInterfaceAddress(Interface) : nullptr;

            if (InterfaceObject && Anchor::At(Scanner::Virtual<const void*>(InterfaceObject, Slot)).Is(Instructions))
            {
                Original = Anchor::DetourVTable(nullptr, *(void***)InterfaceObject, Slot, Detour);
                Classes++;
            }
        }

        if (!bSkipped)
            UE_LOG(LogVelora, Log, TEXT("Hooked %s in %d classes"), this->Name, Classes);

        return Original;
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

    FPatch& Optional()
    {
        bSkipped = !this->IsActive() && !Code.Get();

        return *this;
    }

    const uint8_t* Jump() const
    {
        auto Branch = Code.Get();

        if (bSkipped)
            return nullptr;

        return Branch && *Branch == 0x0F ? Anchor::Patch(Name, Branch, { 0x90, 0xE9 }) : Anchor::Patch(Name, Branch, { 0xEB });
    }

    const uint8_t* JumpOver(std::string_view Instructions) const
    {
        auto Branch = Code.Get();
        auto Next = Code.Find(Instructions).Find("*").Get();

        if (bSkipped)
            return nullptr;

        return Next ? Anchor::Patch(Name, Branch, { 0xEB, (uint8_t)(Next - (Branch + 2)) }) : Anchor::Patch(Name, nullptr, {});
    }

    const uint8_t* BranchTo(const void* Target) const
    {
        if (bSkipped)
            return nullptr;

        return Anchor::PatchBranch(Name, Code.Get(), (const uint8_t*)Target);
    }

    const uint8_t* BranchToLast(std::string_view Instructions) const
    {
        if (bSkipped)
            return nullptr;

        return Anchor::PatchBranch(Name, Code.Get(), Code.Last(Instructions).Target());
    }

private:
    bool bSkipped = false;
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

template<typename FunctionType>
FunctionType DetourAs(const void* Address)
{
    FunctionType Function = {};
    memcpy(&Function, &Address, sizeof(Address));

    return Function;
}

struct FDetourEntry
{
    uint64_t Hash;
    void (*Install)();
    FDetourEntry* Next;
    bool bInstalled;
};

inline FDetourEntry* DetourEntries = nullptr;
inline FDetourEntry** DetourTail = &DetourEntries;
inline bool bDetoursInstalled = false;

template<uint64_t Hash>
inline const void* DetourAddresses = nullptr;

inline void DetourInstall(uint64_t Hash)
{
    for (auto Entry = DetourEntries; Entry; Entry = Entry->Next)
    {
        if (Entry->bInstalled || Entry->Hash != Hash)
            continue;

        Entry->bInstalled = true;
        Entry->Install();
    }
}

inline void DetourInstall()
{
    for (auto Entry = DetourEntries; Entry; Entry = Entry->Next)
        DetourInstall(Entry->Hash);

    bDetoursInstalled = true;
}

template<uint64_t Hash>
const void* DetourOriginalOf()
{
    if (!bDetoursInstalled)
        DetourInstall(Hash);

    return DetourAddresses<Hash>;
}

template<typename ClassType, auto Name, auto Function>
struct TDetourAnchor
{
    using Address = ::Anchor::Address;
    using Offset = ::Anchor::Offset;

    static auto Hook()
    {
        return THook(Name(), Function(std::type_identity<ClassType>()));
    }

    template<typename FunctionType>
    static auto Hook(FunctionType Detour)
    {
        return THook(Name(), Detour);
    }

    template<typename FunctionType>
    static auto Hook(const wchar_t* InName, FunctionType Detour)
    {
        return THook(InName, Detour);
    }
};

template<uint64_t Hash, auto Install>
struct TDetour
{
    static void Run()
    {
        if (auto Address = DetourAddress(Install()))
            DetourAddresses<Hash> = Address;
    }

    static auto Get()
    {
        (void)*(const volatile bool*)&bRegistered;

        return DetourAs<decltype(Install())>(DetourAddresses<Hash>);
    }

    static inline FDetourEntry Entry = { Hash, &Run, nullptr, false };
    static inline const bool bRegistered = (*DetourTail = &Entry, DetourTail = &Entry.Next, true);
};

#define DETOUR_LOCAL(Line) DETOUR_LOCAL_LINE(Line)
#define DETOUR_LOCAL_LINE(Line) DetourOriginal##Line

#define DETOUR(Class, Function, ...) \
    __pragma(comment(linker, "/include:" __FUNCDNAME__)) \
    [[maybe_unused]] const auto DETOUR_LOCAL(__LINE__) = TDetour<DetourHash(#Class "::" #Function), [] \
    { \
        using Anchor = TDetourAnchor<Class, [] { return TEXT(#Class) TEXT("::") TEXT(#Function); }, [](auto Type) { return &decltype(Type)::type::Function; }>; \
        \
        return (__VA_ARGS__); \
    }>::Get(); \
    using DetourType = std::remove_const_t<decltype(DETOUR_LOCAL(__LINE__))>

#define DETOUR_SCOPE(Class, Function, Line) DETOUR_SCOPE_LINE(Class, Function, Line)
#define DETOUR_SCOPE_LINE(Class, Function, Line) Class##_##Function##_Detour##Line

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

#define Original(...) DetourBind(DetourAs<DetourType>(DetourAddresses<DetourHash(__FUNCTION__)>), this)(__VA_ARGS__)

#define ORIGINAL_OF(Class, Function) DetourBind(DetourAs<DetourType>(DETOUR_ORIGINAL(Class, Function)), this)

#define DETOUR_ORIGINAL(Class, Function) DetourOriginalOf<DetourHash(#Class "::" #Function)>()
