#include "pch.h"
#include "FortDagwoodVehicleManager.h"
#include "FortDagwoodVehicle.h"
#include "Items/FortVehicleItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortDagwoodVehicleManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager"));

    return Class;
}

void UFortDagwoodVehicleManager::OnVehicleSpawnsUpdated(UFortVehicleItemDefinition* VehicleItemDef, TArray<AActor*>& SpawnedVehicles, int32 NewVehicleCount)
{
    DETOUR(UFortDagwoodVehicleManager, OnVehicleSpawnsUpdated, Anchor::Hook()
        .Native(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:OnVehicleSpawnsUpdated"))
        .Optional()
        .Inline());

    static auto AllowInoperableVehicles = Anchor::Data<int32*>(TEXT("FortValetCVars::AllowInoperableVehicles"))
        .String(TEXT("Athena.AllowInoperableVehicles"))
        .Last("lea r8, [0x*]")
        .Target()
        .Get();
    static auto OverridePercentInoperable = Anchor::Data<float*>(TEXT("FortValetCVars::OverridePercentInoperable"))
        .String(TEXT("Athena.Valet.OverridePercentInoperable"))
        .Last("lea r8, [0x*]")
        .Target()
        .Get();

    auto VehicleClass = VehicleItemDef->GetVehicleActorClass().LoadSynchronous();

    if (!VehicleClass || !VehicleClass->IsChildOf(AActor::StaticClass()) || !VehicleClass->IsChildOf(AFortDagwoodVehicle::StaticClass()))
        return;

    const float MinPercentInoperable = VehicleItemDef->GetMinPercentInoperable().GetValueAtLevel(0.0f);
    const float MaxPercentInoperable = VehicleItemDef->GetMaxPercentInoperable().GetValueAtLevel(0.0f);
    float PercentInoperable = OverridePercentInoperable ? *OverridePercentInoperable : -1.0f;

    if (!(PercentInoperable >= 0.0f && PercentInoperable <= 1.0f))
        PercentInoperable = FMath::Lerp(MinPercentInoperable, MaxPercentInoperable, FMath::FRand()) * 0.01f;

    const int32 NumInoperable = AllowInoperableVehicles && *AllowInoperableVehicles ? (int32)(NewVehicleCount * PercentInoperable) : 0;
    int32 NumToMakeInoperable = NumInoperable;

    for (auto SpawnedVehicle : SpawnedVehicles)
    {
        if (NumToMakeInoperable <= 0)
            break;

        if (auto Vehicle = Cast<AFortDagwoodVehicle>(SpawnedVehicle))
        {
            Vehicle->MakeInoperable();
            NumToMakeInoperable--;
        }
    }

    const float MinPercentWithGas = VehicleItemDef->GetMinPercentWithGas().GetValueAtLevel(0.0f);
    const float MaxPercentWithGas = VehicleItemDef->GetMaxPercentWithGas().GetValueAtLevel(0.0f);
    const float PercentWithGas = FMath::Lerp(MinPercentWithGas, MaxPercentWithGas, FMath::FRand()) * 0.01f;
    int32 NumWithGas = (int32)((NewVehicleCount - NumInoperable) * PercentWithGas);

    for (auto SpawnedVehicle : SpawnedVehicles)
    {
        auto Vehicle = Cast<AFortDagwoodVehicle>(SpawnedVehicle);

        if (!Vehicle || Vehicle->IsInoperable())
            continue;

        if (NumWithGas <= 0)
            break;

        NumWithGas--;
        Vehicle->AddSpawningFuel();
    }
}
