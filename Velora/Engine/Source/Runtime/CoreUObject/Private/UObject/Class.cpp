#include "pch.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UProperty* UStruct::FindPropertyByName(FName InName) const
{
    static auto FindPropertyByName = (UProperty* (*)(const UStruct*, FName))Scanner::Pattern("48 8B 41 58 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 18 74 ? 48 8B 40 50 48 85 C0 75 ? 33 C0 C3").Get();

    return FindPropertyByName(this, InName);
}

Native UFunction::GetNativeFunc() const
{
    static auto Func = Scanner::String(TEXT("Failed to bind native function %s.%s")).Start().Find("mov [*+0x*], rax").Disp();

    return *(Native*)((uint8_t*)this + Func);
}

UObject* UClass::GetDefaultObject(bool bCreateIfNeeded) const
{
    static auto ClassDefaultObject = Scanner::String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'.")).Start().Find("cmp qword ptr [*+0x*], 0x0").Disp();

    if (*(UObject**)((uint8_t*)this + ClassDefaultObject) == nullptr && bCreateIfNeeded)
    {
        InternalCreateDefaultObjectWrapper();
    }

    return *(UObject**)((uint8_t*)this + ClassDefaultObject);
}

UObject* UClass::InternalCreateDefaultObjectWrapper() const
{
    static auto CreateDefaultObject = Scanner::String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'.")).Start().Find("cmp qword ptr [*+0x*], 0x0").Find("call [*+0x*]").Disp();

    auto VTable = *(uint8_t**)this;

    return (*(UObject* (**)(const UClass*))(VTable + CreateDefaultObject))(this);
}

UClass* UClass::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Class"));

    return Class;
}
