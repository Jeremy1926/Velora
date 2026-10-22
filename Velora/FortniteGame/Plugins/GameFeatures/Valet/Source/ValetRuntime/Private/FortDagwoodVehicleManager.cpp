#include "pch.h"
#include "FortDagwoodVehicleManager.h"
#include "FortDagwoodVehicle.h"
#include "FortValetGameFeatureData.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Building/BuildingActor.h"
#include "Items/FortVehicleItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "Engine/EngineTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/TransformVectorized.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectArray.h"
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

void UFortDagwoodVehicleManager::UpdatePropReplacement()
{
    static auto bHasUpdatedPropReplacement = StaticClass()->FindPropertyByName(FName(TEXT("bHasUpdatedPropReplacement")));

    auto Owner = GetOwner();
    auto& bHasUpdated = *bHasUpdatedPropReplacement->ContainerPtrToValuePtr<bool>(this);

    if (bHasUpdated || !Owner || !Owner->HasAuthority())
        return;

    bHasUpdated = true;

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        auto ObjectItem = GUObjectArray.IndexToObject(Index);
        auto GameFeatureData = ObjectItem ? Cast<UFortValetGameFeatureData>((UObject*)ObjectItem->Object) : nullptr;

        if (!GameFeatureData)
            continue;

        auto VehicleSpawnerClass = GameFeatureData->GetVehicleSpawnerClass().LoadSynchronous();

        for (auto& PropReplacement : GameFeatureData->GetPropReplacements())
        {
            auto PropClass = PropReplacement.Key.LoadSynchronous();

            if (!PropClass)
                continue;

            TArray<AActor*> BuildingProps;
            UGameplayStatics::GetAllActorsOfClass(Owner, PropClass, BuildingProps);

            for (auto BuildingProp : BuildingProps)
                SwapOutProp((ABuildingActor*)BuildingProp, VehicleSpawnerClass, PropReplacement.Value, GameFeatureData->GetSpawnActorCollisionHandlingMethod());
        }
    }
}

void UFortDagwoodVehicleManager::execUpdatePropReplacement(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortDagwoodVehicleManager, execUpdatePropReplacement, Anchor::Hook()
        .Exec(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:UpdatePropReplacement"))
        .Find("mov [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:UpdatePropReplacement"))
        .Optional()
        .InExec(TEXT("/Script/ValetRuntime.FortDagwoodVehicleManager:UpdatePropReplacement")));

    Stack.Code += !!Stack.Code;

    UpdatePropReplacement();
}

AFortAthenaVehicleSpawner* UFortDagwoodVehicleManager::SwapOutProp(ABuildingActor* BuildingProp, UClass* VehicleSpawnerClass, const FVehiclePropReplacementData& PropReplacementData, ESpawnActorCollisionHandlingMethod SpawnActorCollisionHandlingMethod)
{
    auto VehicleItemDef = PropReplacementData.FortVehicleItemDefinition.LoadSynchronous();

    if (!BuildingProp || !VehicleSpawnerClass || !VehicleItemDef)
        return nullptr;

    const FTransform SpawnTransform = UKismetMathLibrary::ComposeTransforms(FTransform(PropReplacementData.RotOffset, PropReplacementData.TransOffset, FVector(1.0f, 1.0f, 1.0f)), BuildingProp->GetTransform());

    BuildingProp->SilentDie(true);

    auto VehicleSpawner = Cast<AFortAthenaVehicleSpawner>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, VehicleSpawnerClass, SpawnTransform, SpawnActorCollisionHandlingMethod));

    if (!VehicleSpawner)
        return nullptr;

    VehicleSpawner->SetFortVehicleItemDef(VehicleItemDef);

    return Cast<AFortAthenaVehicleSpawner>(UGameplayStatics::FinishSpawningActor(VehicleSpawner, SpawnTransform));
}
