#include "pch.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAbilitySystemComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySystemComponent"));

    return Class;
}
