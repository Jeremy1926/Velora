#pragma once

#include "Components/ActorComponent.h"
#include "UObject/NameTypes.h"

class AController;
class AFortPawn;
class AFortPlayerController;
class UCurveTable;
class UFortResourceItemDefinition;
struct FScalableFloat;

class UFortControllerComponent_InventoryService : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UCurveTable* GetDefaultCurrencyPayoutTable();

    static UFortResourceItemDefinition* GetDefaultGlobalCurrencyItemDefinition();

    static UFortControllerComponent_InventoryService* GetInventoryServiceComponent(AFortPlayerController* FortPC);

    static int32 GetCurrencyPayoutsValueForRow(const FName& RowName, float EvalInput);

    static void GenerateCurrencyOnPawnDeath(AFortPawn* KilledPawn, AController* KillerController, const FName& CurrencyPayoutRowName);

    void GiveGlobalCurrency(int32 Amount);

    FScalableFloat& GetShouldDropCurrencyOnDeath();

    int32& GetTotalCurrencyEarnedInMatch();
};
