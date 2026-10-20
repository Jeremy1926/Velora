#pragma once

#include "Abilities/FortGameplayAbility.h"

class AFortPlayerController;
class UFortWorldItemDefinition;
class UScriptStruct;
struct FFrame;
struct FScalableFloat;

struct FActiveItemGrantInfo
{
    static UScriptStruct* StaticStruct();

    UFortWorldItemDefinition* GetItem() const;

    const FScalableFloat& GetAmountToGive() const;

    const FScalableFloat& GetMaxAmount() const;
};

class UFortGameplayAbilityAthena_PeriodicItemGrant : public UFortGameplayAbility
{
public:
    static UClass* StaticClass();

    void StartItemAwardTimers();

    void StopItemAwardTimers();

    void AwardItem(const FActiveItemGrantInfo& GrantInfo);

    void execStartItemAwardTimers(FFrame& Stack, void* const Z_Param__Result);

    void execStopItemAwardTimers(FFrame& Stack, void* const Z_Param__Result);

    static void TickItemAwardTimers();

    AFortPlayerController*& GetOwnerControllerCachedValue();
};
