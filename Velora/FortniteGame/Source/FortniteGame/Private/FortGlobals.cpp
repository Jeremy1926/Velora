#include "pch.h"
#include "FortGlobals.h"
#include "FortGameData.h"
#include "FortniteGame.h"
#include "Abilities/FortAbilitySet.h"
#include "Logging/LogMacros.h"

void UFortGlobals::InitializePlayerGameplayAbilities(IAbilitySystemInterface* PlayerStateOrProxy)
{
    if (!PlayerStateOrProxy)
    {
        UE_LOG(LogFort, Error, TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"));
        return;
    }

    if (auto AbilitySet = UFortGameData::Get()->GetGenericPlayerAbilitySet())
        AbilitySet->EquipAbilitySet(PlayerStateOrProxy, nullptr, nullptr);
}
