#include "pch.h"
#include "Components/FortControllerComponent_InventoryService.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "Items/FortResourceItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Player/FortPlayerController.h"
#include "Engine/CurveTable.h"
#include "GameFramework/Controller.h"
#include "HAL/IConsoleManager.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortControllerComponent_InventoryService::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_InventoryService"));

    return Class;
}

UCurveTable* UFortControllerComponent_InventoryService::GetDefaultCurrencyPayoutTable()
{
    static auto GetDefaultCurrencyPayoutTable = Anchor::Optional<UCurveTable* (*)()>(TEXT("UFortControllerComponent_InventoryService::GetDefaultCurrencyPayoutTable"))
        .Native(TEXT("/Script/FortniteGame.FortControllerComponent_InventoryService:GetDefaultCurrencyPayoutTable"))
        .Get();

    return GetDefaultCurrencyPayoutTable ? GetDefaultCurrencyPayoutTable() : nullptr;
}

UFortResourceItemDefinition* UFortControllerComponent_InventoryService::GetDefaultGlobalCurrencyItemDefinition()
{
    static auto GetDefaultGlobalCurrencyItemDefinition = Anchor::Optional<UFortResourceItemDefinition* (*)()>(TEXT("UFortControllerComponent_InventoryService::GetDefaultGlobalCurrencyItemDefinition"))
        .Native(TEXT("/Script/FortniteGame.FortControllerComponent_InventoryService:GetDefaultGlobalCurrencyItemDefinition"))
        .Get();

    return GetDefaultGlobalCurrencyItemDefinition ? GetDefaultGlobalCurrencyItemDefinition() : nullptr;
}

UFortControllerComponent_InventoryService* UFortControllerComponent_InventoryService::GetInventoryServiceComponent(AFortPlayerController* FortPC)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_InventoryService:GetInventoryServiceComponent"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
        return FortPC ? (UFortControllerComponent_InventoryService*)FortPC->FindComponentByClass(StaticClass()) : nullptr;

    static auto GetInventoryServiceComponent = Anchor::Optional<UFortControllerComponent_InventoryService* (*)(AFortPlayerController*)>(TEXT("UFortControllerComponent_InventoryService::GetInventoryServiceComponent"))
        .Native(TEXT("/Script/FortniteGame.FortControllerComponent_InventoryService:GetInventoryServiceComponent"))
        .Get();

    return GetInventoryServiceComponent && FortPC ? GetInventoryServiceComponent(FortPC) : nullptr;
}

int32 UFortControllerComponent_InventoryService::GetCurrencyPayoutsValueForRow(const FName& RowName, float EvalInput)
{
    auto CurrencyPayoutTable = GetDefaultCurrencyPayoutTable();

    if (!CurrencyPayoutTable)
        return 0;

    static auto Value = FScalableFloat::StaticStruct()->FindPropertyByName(FName(TEXT("Value")));
    static auto Curve = FScalableFloat::StaticStruct()->FindPropertyByName(FName(TEXT("Curve")));

    TArray<uint8> CurrencyPayout;
    CurrencyPayout.AddUninitialized(FScalableFloat::StaticStruct()->GetPropertiesSize());
    memset(CurrencyPayout.GetData(), 0, CurrencyPayout.Num());

    auto& CurrencyPayoutCurve = *Curve->ContainerPtrToValuePtr<FCurveTableRowHandle>(CurrencyPayout.GetData());

    *Value->ContainerPtrToValuePtr<float>(CurrencyPayout.GetData()) = 1.0f;
    CurrencyPayoutCurve = FCurveTableRowHandle{ CurrencyPayoutTable, RowName };

    const float Payout = ((FScalableFloat*)CurrencyPayout.GetData())->GetValueAtLevel(EvalInput);

    return *(const void* const*)(&CurrencyPayoutCurve + 1) ? (int32)Payout : 0;
}

void UFortControllerComponent_InventoryService::GenerateCurrencyOnPawnDeath(AFortPawn* KilledPawn, AController* KillerController, const FName& CurrencyPayoutRowName)
{
    static auto DropCurrencyOnPawnDeath = IConsoleManager::Get().FindConsoleVariable(TEXT("Athena.DropCurrencyOnPawnDeath"));

    auto GlobalCurrencyItemDefinition = GetDefaultGlobalCurrencyItemDefinition();
    const int32 Amount = DropCurrencyOnPawnDeath && GlobalCurrencyItemDefinition ? GetCurrencyPayoutsValueForRow(CurrencyPayoutRowName, 1.0f) : 0;

    if (!KilledPawn || Amount <= 0)
        return;

    if (DropCurrencyOnPawnDeath->GetInt())
    {
        if (auto InventoryService = GetInventoryServiceComponent(Cast<AFortPlayerController>(KillerController)))
            InventoryService->GiveGlobalCurrency(Amount);

        return;
    }

    UFortKismetLibrary::K2_SpawnPickupInWorld(KilledPawn, GlobalCurrencyItemDefinition, Amount, KilledPawn->GetActorLocation(), FVector(0.0f, 0.0f, 0.0f), 0, true);
}

void UFortControllerComponent_InventoryService::GiveGlobalCurrency(int32 Amount)
{
    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto GlobalCurrencyItemDefinition = GetDefaultGlobalCurrencyItemDefinition();

    if (!PlayerController || !GlobalCurrencyItemDefinition || Amount <= 0)
        return;

    UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, GlobalCurrencyItemDefinition, Amount);

    GetTotalCurrencyEarnedInMatch() += Amount;
}

FScalableFloat& UFortControllerComponent_InventoryService::GetShouldDropCurrencyOnDeath()
{
    static auto ShouldDropCurrencyOnDeath = GetClass()->FindPropertyByName(FName(TEXT("ShouldDropCurrencyOnDeath")));

    return *ShouldDropCurrencyOnDeath->ContainerPtrToValuePtr<FScalableFloat>(this);
}

int32& UFortControllerComponent_InventoryService::GetTotalCurrencyEarnedInMatch()
{
    static auto TotalCurrencyEarnedData = GetClass()->FindPropertyByName(FName(TEXT("TotalCurrencyEarnedData")));
    static auto TotalCurrencyEarnedInMatch = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.GlobalCurrencyTrackedData")))->FindPropertyByName(FName(TEXT("TotalCurrencyEarnedInMatch")));

    return *TotalCurrencyEarnedInMatch->ContainerPtrToValuePtr<int32>(TotalCurrencyEarnedData->ContainerPtrToValuePtr<void>(this));
}
