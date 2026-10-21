#include "pch.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FFortAbilitySetHandle::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySetHandle"));

    return Struct;
}

int32 FFortAbilitySetHandle::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

UClass* UFortAbilitySet::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySet"));

    return Class;
}

void UFortAbilitySet::UnequipAbilitySet(FFortAbilitySetHandle& Handle)
{
    static auto UnequipAbilitySet = Anchor::Function<void (*)(FFortAbilitySetHandle&)>(TEXT("UFortAbilitySet::UnequipAbilitySet"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:UnequipFortAbilitySet"))
        .Get();

    UnequipAbilitySet(Handle);
}

TArray<TSubclassOf<UFortGameplayAbility>>& UFortAbilitySet::GetGameplayAbilities()
{
    static auto GameplayAbilities = GetClass()->FindPropertyByName(FName(TEXT("GameplayAbilities")));

    return *GameplayAbilities->ContainerPtrToValuePtr<TArray<TSubclassOf<UFortGameplayAbility>>>(this);
}
