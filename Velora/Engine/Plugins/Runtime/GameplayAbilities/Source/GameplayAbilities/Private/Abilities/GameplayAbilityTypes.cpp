#include "pch.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

FGameplayEventData::FGameplayEventData()
{
    FMemory::Memzero(this, sizeof(FGameplayEventData));

    StaticStruct()->InitializeStruct(this);
}

FGameplayEventData::~FGameplayEventData()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FGameplayEventData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayEventData"));

    return Struct;
}
