#include "pch.h"
#include "GameplayAbilitySpec.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UScriptStruct* FGameplayAbilitySpec::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayAbilitySpec"));

    return Struct;
}

int32 FGameplayAbilitySpec::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

UScriptStruct* FGameplayAbilitySpecContainer::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayAbilitySpecContainer"));

    return Struct;
}

TRuntimeSizedArray<FGameplayAbilitySpec>& FGameplayAbilitySpecContainer::GetItems()
{
    static auto Items = StaticStruct()->FindPropertyByName(FName(TEXT("Items")));

    return *Items->ContainerPtrToValuePtr<TRuntimeSizedArray<FGameplayAbilitySpec>>(this);
}

void FGameplayAbilitySpecHandle::GenerateNewHandle()
{
    static auto& GHandle = *(int32*)Scanner::String(TEXT("FGameplayAbilitySpec::FGameplayAbilitySpec for %s from %s")).Start().Find("mov eax, [0x*]").Target(TEXT("FGameplayAbilitySpecHandle::GenerateNewHandle::GHandle"));

    Handle = GHandle++;
}
