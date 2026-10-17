#include "pch.h"
#include "AbilitySystemInterface.h"
#include "UObject/UObjectGlobals.h"

UClass* UAbilitySystemInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemInterface"));

    return Class;
}
