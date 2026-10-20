#include "pch.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UProperty* UStruct::FindPropertyByName(FName InName) const
{
    auto& Version = FEngineVersion::Current();

    static auto FindPropertyByName = Anchor::Function<UProperty* (*)(const UStruct*, FName)>(TEXT("UStruct::FindPropertyByName"))
        .If(Version.GetMajor() > 4 || Version.GetMinor() >= 25)
        .Pattern("48 8B 49 70 48 85 C9 74 ? 4C 8B CA 49 C1 E9 20 48 8B 41 28")
        .Or()
        .If(Version.GetMajor() <= 4 && Version.GetMinor() >= 22 && Version.GetMinor() < 25)
        .Pattern("48 8B 41 68 48 89 54 24 10 48 85 C0 74 ? 44 8B 54 24 14")
        .Or()
        .If(Version.GetMajor() <= 4 && Version.GetMinor() < 22)
        .Pattern("48 8B 41 58 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 18 74 ? 48 8B 40 50 48 85 C0 75 ? 33 C0 C3")
        .Get();

    return FindPropertyByName(this, InName);
}

void UStruct::InitializeStruct(void* Dest, int32 ArrayDim) const
{
    static auto InitializeStruct = Anchor::Slot(TEXT("UStruct::InitializeStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r8d, 0x1")
        .Find("call [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(const UStruct*, void*, int32)>(this, InitializeStruct)(this, Dest, ArrayDim);
}

void UStruct::DestroyStruct(void* Dest, int32 ArrayDim) const
{
    static auto DestroyStruct = Anchor::Slot(TEXT("UStruct::InitializeStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r8d, 0x1")
        .Find("call [*+0x*]")
        .Get() + (int32)sizeof(void*);

    Scanner::Virtual<void (*)(const UStruct*, void*, int32)>(this, DestroyStruct)(this, Dest, ArrayDim);
}

void UScriptStruct::CopyScriptStruct(void* Dest, void const* Src, int32 ArrayDim) const
{
    static auto CopyScriptStruct = Anchor::Function<void (*)(const UScriptStruct*, void*, void const*, int32)>(TEXT("UScriptStruct::CopyScriptStruct"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:FilterTargetData"))
        .Find("mov r9d, 0x1")
        .Find("call 0x*")
        .Target()
        .Get();

    CopyScriptStruct(this, Dest, Src, ArrayDim);
}

static int32 FuncOffset()
{
    static auto Func = Scanner::String(TEXT("Failed to bind native function %s.%s")).Start().Find("mov [*+0x*], rax").Disp(TEXT("UFunction::Func"));

    return Func;
}

Native UFunction::GetNativeFunc() const
{
    return *(Native*)((uint8_t*)this + FuncOffset());
}

void UFunction::SetNativeFunc(Native InFunc)
{
    *(Native*)((uint8_t*)this + FuncOffset()) = InFunc;
}

int64 UEnum::GetValueByName(FName InName) const
{
    for (auto& Kvp : Names)
    {
        if (Kvp.Key == InName)
            return Kvp.Value;
    }

    return -1;
}

UObject* UClass::GetDefaultObject(bool bCreateIfNeeded) const
{
    static auto ClassDefaultObject = Scanner::String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'.")).Start().Find("call [rax+0x*]; mov *, [*+0x*]").Find("mov *, [*+0x*]").Disp(TEXT("UClass::ClassDefaultObject"));

    if (*(UObject**)((uint8_t*)this + ClassDefaultObject) == nullptr && bCreateIfNeeded)
    {
        InternalCreateDefaultObjectWrapper();
    }

    return *(UObject**)((uint8_t*)this + ClassDefaultObject);
}

UObject* UClass::InternalCreateDefaultObjectWrapper() const
{
    static auto CreateDefaultObject = Anchor::Slot(TEXT("UClass::CreateDefaultObject"))
        .String(TEXT("Tried to UFortAIGoalProvider_EnvQuery::GetAssociatedPawnDefaultObject() on an unloaded pawn class '%s' in query '%s'."))
        .Start()
        .Find("call [rax+0x*]; mov *, [*+0x*]")
        .Get();

    return Scanner::Virtual<UObject* (*)(const UClass*)>(this, CreateDefaultObject)(this);
}

UClass* UClass::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Class"));

    return Class;
}
