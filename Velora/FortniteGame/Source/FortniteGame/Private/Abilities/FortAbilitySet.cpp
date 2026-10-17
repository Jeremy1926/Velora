#include "pch.h"
#include "Abilities/FortAbilitySet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAbilitySet::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySet"));

    return Class;
}

TArray<TSubclassOf<UFortGameplayAbility>>& UFortAbilitySet::GetGameplayAbilities()
{
    static auto GameplayAbilities = GetClass()->FindPropertyByName(FName(TEXT("GameplayAbilities")));

    return *GameplayAbilities->ContainerPtrToValuePtr<TArray<TSubclassOf<UFortGameplayAbility>>>(this);
}
