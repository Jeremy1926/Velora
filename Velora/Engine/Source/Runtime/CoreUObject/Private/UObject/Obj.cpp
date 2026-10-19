#include "pch.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UObject::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Object"));

    return Class;
}

UWorld* UObject::GetWorld() const
{
    static auto GetWorld = Scanner::String(TEXT("Changing GameSessionId from '%s' to '%s'")).Start().Find("call [*+0x*]").Slot(TEXT("UObject::GetWorld"));

    auto VTable = *(uint8_t**)this;

    return (*(UWorld* (**)(const UObject*))(VTable + GetWorld))(this);
}

UObject* UObject::GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags)
{
    static auto GetArchetypeFromRequiredInfo = Scanner::Function<UObject* (*)(UClass*, UObject*, FName, EObjectFlags)>([]
    {
        auto Archetype = Scanner::String(TEXT("Component archetype is not the CDO nor a default subobject of my class. Me = %s, Component = %s, Archetype = %s")).Last("xor r14b, r14b").Call().Find("mov r9d, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*");

        if (!Archetype.Get())
            Archetype = Scanner::Pattern("44 8B 4F 08 4C 8B 47 18 48 8B 57 20 48 8B 4F 10 E8");

        if (!Archetype.Get())
            return Scanner::String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead.")).Call(2).Call().Get(TEXT("UObject::GetArchetypeFromRequiredInfo"));

        return Archetype.Call().Get(TEXT("UObject::GetArchetypeFromRequiredInfo"));
    }());

    return GetArchetypeFromRequiredInfo(Class, Outer, Name, ObjectFlags);
}

bool UObject::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
    static auto CallRemoteFunction = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Default__Object"));
        auto Index = Scanner::String(TEXT("NotifyEventTriggered: Unable to find function '%s'")).Start().Find("call [*+0x*]").Disp() / sizeof(void*);

        return Scanner::At(VTable[Index]).Find("test dword ptr [*], 0x400").Find("call [*+0x*]", 2).Slot(TEXT("UObject::CallRemoteFunction"));
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(UObject*, UFunction*, void*, FOutParmRec*, FFrame*))(VTable + CallRemoteFunction))(this, Function, Parms, OutParms, Stack);
}
