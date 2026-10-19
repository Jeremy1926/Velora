#include "pch.h"
#include "Abilities/FortGameplayAbility.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortGameplayAbility::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbility"));

    return Class;
}
