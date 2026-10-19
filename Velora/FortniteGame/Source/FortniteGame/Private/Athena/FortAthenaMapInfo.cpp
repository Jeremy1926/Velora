#include "pch.h"
#include "Athena/FortAthenaMapInfo.h"
#include "GameplayEffectTypes.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortAthenaMapInfo::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMapInfo"));

    return Class;
}

void AFortAthenaMapInfo::SpawnLlamas()
{
    auto LlamaClass = GetLlamaClass();
    auto SafeZoneRadius = GetSafeZoneRadius();
    auto SupplyDropMinPlacementHeight = GetSupplyDropMinPlacementHeight();
    auto SupplyDropMaxPlacementHeight = GetSupplyDropMaxPlacementHeight();

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
        const FVector SpawnLocation = PickSupplyDropLocation(MapCenter, Radius);
        const FRotator SpawnRotation{ 0.0f, FMath::FRand() * 360.0f, 0.0f };

        SpawnSupplyDropAtLocation(SpawnLocation, SpawnRotation, LlamaClass, TraceStartZ, TraceEndZ);
    }
}

FVector AFortAthenaMapInfo::PickSupplyDropLocation(const FVector& Center, float Radius) const
{
    static auto PickSupplyDropLocation = Scanner::Function<FVector* (*)(const AFortAthenaMapInfo*, FVector*, const FVector&, float)>(Scanner::String(TEXT("PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location.")).Start().Get(TEXT("AFortAthenaMapInfo::PickSupplyDropLocation")));

    FVector Location;

    return *PickSupplyDropLocation(this, &Location, Center, Radius);
}

AFortAthenaSupplyDrop* AFortAthenaMapInfo::SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const
{
    static auto SpawnSupplyDropAtLocation = Scanner::Function<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const FVector&, const FRotator&, UClass*, float, float)>(Scanner::CallTo(Scanner::String(TEXT("PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location.")).Start().Get()).Find("mov r9, [*+0x*]").Find("call 0x*").Call().Get(TEXT("AFortAthenaMapInfo::SpawnSupplyDropAtLocation")));

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
    static auto Radius = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition")))->FindPropertyByName(FName(TEXT("Radius")))->GetOffset_ForInternal();

    auto& Definitions = *SafeZoneDefinitions->ContainerPtrToValuePtr<TArray<uint8>>(this);

    return Definitions.Num() ? (const FScalableFloat*)(&Definitions[0] + Radius) : nullptr;
}
