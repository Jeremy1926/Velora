#include "pch.h"
#include "Athena/FortAthenaMapInfo.h"
#include "GameplayEffectTypes.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FVehicleClassDetails::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.VehicleClassDetails"));

    return Struct;
}

int32 FVehicleClassDetails::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

TAssetPtr<UClass>& FVehicleClassDetails::GetVehicleClass()
{
    static auto VehicleClass = StaticStruct()->FindPropertyByName(FName(TEXT("VehicleClass")));

    return *VehicleClass->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this);
}

const FScalableFloat& FVehicleClassDetails::GetVehicleMinSpawnPercent()
{
    static auto VehicleMinSpawnPercent = StaticStruct()->FindPropertyByName(FName(TEXT("VehicleMinSpawnPercent")));

    return *VehicleMinSpawnPercent->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& FVehicleClassDetails::GetVehicleMaxSpawnPercent()
{
    static auto VehicleMaxSpawnPercent = StaticStruct()->FindPropertyByName(FName(TEXT("VehicleMaxSpawnPercent")));

    return *VehicleMaxSpawnPercent->ContainerPtrToValuePtr<FScalableFloat>(this);
}

UClass* AFortAthenaMapInfo::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMapInfo"));

    return Class;
}

TArray<FVehicleClassDetails>& AFortAthenaMapInfo::GetVehicleClassDetails()
{
    static auto VehicleClassDetails = GetClass()->FindPropertyByName(FName(TEXT("VehicleClassDetails")));

    return *VehicleClassDetails->ContainerPtrToValuePtr<TArray<FVehicleClassDetails>>(this);
}

void AFortAthenaMapInfo::SpawnLlamas()
{
    auto LlamaClass = GetLlamaClass();
    auto SafeZoneRadius = GetSafeZoneRadius();
    auto SupplyDropMinPlacementHeight = GetSupplyDropMinPlacementHeight();
    auto SupplyDropMaxPlacementHeight = GetSupplyDropMaxPlacementHeight();
    auto DisableSupplyDropsSpawningOverWater = IConsoleManager::Get().FindConsoleVariable(TEXT("Athena.DisableSupplyDropsSpawningOverWater"));

    if (!LlamaClass || !SafeZoneRadius)
        return;

    const int32 LlamaQuantityMin = FMath::TruncToInt(GetLlamaQuantityMin().GetValueAtLevel(0.0f));
    const int32 LlamaQuantityRange = FMath::TruncToInt(GetLlamaQuantityMax().GetValueAtLevel(0.0f)) - LlamaQuantityMin;
    const int32 LlamaQuantity = LlamaQuantityMin + (LlamaQuantityRange + 1 > 0 ? FMath::Min(FMath::TruncToInt(FMath::FRand() * (LlamaQuantityRange + 1)), LlamaQuantityRange) : 0);
    const FVector MapCenter = GetActorLocation();
    const float Radius = SafeZoneRadius->GetValueAtLevel(0.0f);
    const float TraceStartZ = SupplyDropMaxPlacementHeight ? SupplyDropMaxPlacementHeight->GetValueAtLevel(0.0f) : 0.0f;
    const float TraceEndZ = SupplyDropMinPlacementHeight ? SupplyDropMinPlacementHeight->GetValueAtLevel(0.0f) : 0.0f;

    for (int32 Index = 0; Index < LlamaQuantity; Index++)
    {
        const FVector SpawnLocation = PickSupplyDropLocation(MapCenter, Radius, DisableSupplyDropsSpawningOverWater && DisableSupplyDropsSpawningOverWater->GetInt() == 1, TraceStartZ, TraceEndZ);
        const FRotator SpawnRotation{ 0.0f, FMath::FRand() * 360.0f, 0.0f };

        SpawnSupplyDropAtLocation(SpawnLocation, SpawnRotation, LlamaClass, TraceStartZ, TraceEndZ);
    }
}

FVector AFortAthenaMapInfo::PickSupplyDropLocation(const FVector& Center, float Radius, bool bDisableSupplyDropsSpawningOverWater, float TraceStartZ, float TraceEndZ) const
{
    static auto PickSupplyDropLocation = Anchor::Function<FVector* (*)(const AFortAthenaMapInfo*, FVector*, const FVector&, float, bool, float, float)>(TEXT("AFortAthenaMapInfo::PickSupplyDropLocation"))
        .String(TEXT("PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location."))
        .Start()
        .Get();

    FVector Location;

    return *PickSupplyDropLocation(this, &Location, Center, Radius, bDisableSupplyDropsSpawningOverWater, TraceStartZ, TraceEndZ);
}

AFortAthenaSupplyDrop* AFortAthenaMapInfo::SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const
{
    static auto SpawnSupplyDropAtLocation = Anchor::Function<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const FVector&, const FRotator&, UClass*, float, float)>(TEXT("AFortAthenaMapInfo::SpawnSupplyDropAtLocation"))
        .String(TEXT("SupplyDropSpawn"))
        .Caller()
        .Or()
        .String(TEXT("FortAthenaSupplyDrop"))
        .Start()
        .Reference("call 0x*; test rax, rax; jz *; lea r8, [rax+0x30]")
        .Start()
        .Get();

    return SpawnSupplyDropAtLocation(this, SpawnLocation, SpawnRotation, InSupplyDropClass, TraceStartZ, TraceEndZ);
}

UClass* AFortAthenaMapInfo::GetLlamaClass() const
{
    static auto LlamaClass = GetClass()->FindPropertyByName(FName(TEXT("LlamaClass")));

    return LlamaClass ? *LlamaClass->ContainerPtrToValuePtr<UClass*>(this) : nullptr;
}

const FScalableFloat& AFortAthenaMapInfo::GetLlamaQuantityMin() const
{
    static auto LlamaQuantityMin = GetClass()->FindPropertyByName(FName(TEXT("LlamaQuantityMin")));

    return *LlamaQuantityMin->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMapInfo::GetLlamaQuantityMax() const
{
    static auto LlamaQuantityMax = GetClass()->FindPropertyByName(FName(TEXT("LlamaQuantityMax")));

    return *LlamaQuantityMax->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat* AFortAthenaMapInfo::GetVendingMachineRarityCount() const
{
    static auto VendingMachineRarityCount = GetClass()->FindPropertyByName(FName(TEXT("VendingMachineRarityCount")));

    return VendingMachineRarityCount ? VendingMachineRarityCount->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

const FScalableFloat* AFortAthenaMapInfo::GetWeaponUpgradeMachineSpawnChance() const
{
    static auto WeaponUpgradeMachineSpawnChance = GetClass()->FindPropertyByName(FName(TEXT("WeaponUpgradeMachineSpawnChance")));

    return WeaponUpgradeMachineSpawnChance ? WeaponUpgradeMachineSpawnChance->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

const FScalableFloat* AFortAthenaMapInfo::GetSupplyDropMinPlacementHeight() const
{
    static auto SupplyDropMinPlacementHeight = GetClass()->FindPropertyByName(FName(TEXT("SupplyDropMinPlacementHeight")));

    return SupplyDropMinPlacementHeight ? SupplyDropMinPlacementHeight->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

const FScalableFloat* AFortAthenaMapInfo::GetSupplyDropMaxPlacementHeight() const
{
    static auto SupplyDropMaxPlacementHeight = GetClass()->FindPropertyByName(FName(TEXT("SupplyDropMaxPlacementHeight")));

    return SupplyDropMaxPlacementHeight ? SupplyDropMaxPlacementHeight->ContainerPtrToValuePtr<FScalableFloat>(this) : nullptr;
}

const FScalableFloat* AFortAthenaMapInfo::GetSafeZoneRadius() const
{
    static auto SafeZoneDefinitions = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneDefinitions")));
    static auto SafeZoneDefinition = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneDefinition")));
    static auto Radius = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition")))->FindPropertyByName(FName(TEXT("Radius")))->GetOffset_ForInternal();

    if (!SafeZoneDefinitions)
        return (const FScalableFloat*)(SafeZoneDefinition->ContainerPtrToValuePtr<uint8>(this) + Radius);

    auto& Definitions = *SafeZoneDefinitions->ContainerPtrToValuePtr<TArray<uint8>>(this);

    return Definitions.Num() ? (const FScalableFloat*)(&Definitions[0] + Radius) : nullptr;
}
