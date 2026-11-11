#include "pch.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortAthenaSupplyDrop.h"
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

bool AFortAthenaMapInfo::IsMapInfoValidForAircraft() const
{
    static auto AircraftSpawnZone = GetClass()->FindPropertyByName(FName(TEXT("AircraftSpawnZone")));
    static auto AircraftDropZone = GetClass()->FindPropertyByName(FName(TEXT("AircraftDropZone")));
    static auto AircraftDropVolume = GetClass()->FindPropertyByName(FName(TEXT("AircraftDropVolume")));
    static auto AircraftSpeed = GetClass()->FindPropertyByName(FName(TEXT("AircraftSpeed")));

    auto AircraftClass = GetAircraftClass();

    if (!AircraftClass || !AircraftClass->IsChildOf(AFortAthenaAircraft::StaticClass()))
        return false;

    if (!(AircraftSpeed->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f) > 0.0f))
        return false;

    auto SpawnZone = AircraftSpawnZone->ContainerPtrToValuePtr<double>(this);

    if (!(SpawnZone[2] > SpawnZone[0]) || !(SpawnZone[3] > SpawnZone[1]))
        return false;

    if (*AircraftDropVolume->ContainerPtrToValuePtr<AActor*>(this))
        return true;

    auto DropZone = AircraftDropZone->ContainerPtrToValuePtr<double>(this);

    if (!(DropZone[2] > DropZone[0]) || !(DropZone[3] > DropZone[1]))
        return false;

    auto IsInside = [SpawnZone](const double* Point)
    {
        return Point[0] > SpawnZone[0] && Point[0] < SpawnZone[2] && Point[1] > SpawnZone[1] && Point[1] < SpawnZone[3];
    };

    return IsInside(DropZone) && IsInside(DropZone + 2);
}

void AFortAthenaMapInfo::InitializeFlightPath(AFortGameStateAthena* GameState, UFortGameStateComponent_BattleRoyaleGamePhaseLogic* GamePhaseLogic, bool bForceDropCenterToFirstSafeZone, float PathMidpointSelectionRadiusMin, float PathMidpointSelectionRadiusMax, float LastStepPushCenterLine_Magnitude)
{
    static auto InitializeFlightPath = Anchor::Function<void (*)(AFortAthenaMapInfo*, AFortGameStateAthena*, UFortGameStateComponent_BattleRoyaleGamePhaseLogic*, bool, float, float, float)>(TEXT("AFortAthenaMapInfo::InitializeFlightPath"))
        .String(TEXT("InitializeFlightPath: CustomDropZoneBox2D is invalid when bUseCustomDropZone is true"))
        .Start()
        .Caller()
        .Get();

    InitializeFlightPath(this, GameState, GamePhaseLogic, bForceDropCenterToFirstSafeZone, PathMidpointSelectionRadiusMin, PathMidpointSelectionRadiusMax, LastStepPushCenterLine_Magnitude);
}

FVector AFortAthenaMapInfo::PickSupplyDropLocation(const FVector& Center, float Radius, bool bDisableSupplyDropsSpawningOverWater, float TraceStartZ, float TraceEndZ) const
{
    static auto PickSupplyDropLocation = Anchor::Function<TGameValue<FVector>* (*)(const AFortAthenaMapInfo*, TGameValue<FVector>*, const TGameValue<FVector>&, float, bool, float, float)>(TEXT("AFortAthenaMapInfo::PickSupplyDropLocation"))
        .String(TEXT("PickSupplyDropLocation: Failed to find valid location using rejection.  Using safe zone location."))
        .Start()
        .Or()
        .String("AFortAthenaMapInfo::PickSupplyDropLocation")
        .Start()
        .Get();

    TGameValue<FVector> Location;

    return PickSupplyDropLocation(this, &Location, Center, Radius, bDisableSupplyDropsSpawningOverWater, TraceStartZ, TraceEndZ)->Get();
}

AFortAthenaSupplyDrop* AFortAthenaMapInfo::SpawnSupplyDropAtLocation(const FVector& SpawnLocation, const FRotator& SpawnRotation, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const
{
    static auto SpawnSupplyDropAtLocationByRef = Anchor::Optional<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const TGameValue<FVector>&, const TGameValue<FRotator>&, UClass* const*, float, float)>(TEXT("AFortAthenaMapInfo::SpawnSupplyDropAtLocation"))
        .String(TEXT("AFortAthenaMutator_SupplyDrop::FortGameModeAthena_SpawnItemDeliverySupplyDrop found no locations in safe zone index %i. Locking spawning for %f seconds"))
        .Start()
        .Find("lea r9, [rsp+0x*]; movss [rsp+0x28], xmm*; lea r8, *; lea rdx, *; movss [rsp+0x20], xmm*; call 0x*")
        .Call()
        .Or()
        .String(TEXT("AFortAthenaMutator_SupplyDrop::FortGameModeAthena_SpawnItemDeliverySupplyDrop found no locations in safe zone index %i. Locking spawning for %f seconds"))
        .Start()
        .Find("movss [rsp+0x28], xmm*; lea r9, [rsp+0x*]; lea r8, *; movss [rsp+0x20], xmm*; lea rdx, *; mov rcx, *; call 0x*")
        .Call()
        .Get();

    if (SpawnSupplyDropAtLocationByRef)
        return SpawnSupplyDropAtLocationByRef(this, SpawnLocation, SpawnRotation, &InSupplyDropClass, TraceStartZ, TraceEndZ);

    static auto SpawnSupplyDropAtLocation = Anchor::Function<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const TGameValue<FVector>&, const TGameValue<FRotator>&, UClass*, float, float)>(TEXT("AFortAthenaMapInfo::SpawnSupplyDropAtLocation"))
        .String(TEXT("SupplyDropSpawn"))
        .Caller()
        .Or()
        .String(TEXT("FortAthenaSupplyDrop"))
        .Start()
        .Reference("call 0x*; test rax, rax; jz *; lea r8, [rax+0x30]")
        .Start()
        .Or()
        .String(TEXT("AFortAthenaMutator_SupplyDrop::FortGameModeAthena_SpawnItemDeliverySupplyDrop found no locations in safe zone index %i. Locking spawning for %f seconds"))
        .Start()
        .Find("movss [rsp+0x20], xmm*; mov r9, [r9+0x38]; call 0x*")
        .Call()
        .Get();

    return SpawnSupplyDropAtLocation(this, SpawnLocation, SpawnRotation, InSupplyDropClass, TraceStartZ, TraceEndZ);
}

AFortAthenaSupplyDrop* AFortAthenaMapInfo::SpawnSupplyDropWithinCircle(const TGameValue<FVector>& Center, float Radius, UClass* InSupplyDropClass, float TraceStartZ, float TraceEndZ) const
{
    static auto SpawnSupplyDropWithinCircleByRef = Anchor::Optional<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const TGameValue<FVector>&, float, UClass* const*, float, float)>(TEXT("AFortAthenaMapInfo::SpawnSupplyDropWithinCircle"))
        .String(TEXT("AFortAthenaMutator_Cobalt::SpawnAndRegisterSupplyDrop: Spawned and registered a supply drop"))
        .Start()
        .Find("lea r9, *; movaps xmm2, xmm*; movss [rsp+0x20], xmm*; lea rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (SpawnSupplyDropWithinCircleByRef)
        return SpawnSupplyDropWithinCircleByRef(this, Center, Radius, &InSupplyDropClass, TraceStartZ, TraceEndZ);

    static auto SpawnSupplyDropWithinCircle = Anchor::Function<AFortAthenaSupplyDrop* (*)(const AFortAthenaMapInfo*, const TGameValue<FVector>&, float, UClass*, float, float)>(TEXT("AFortAthenaMapInfo::SpawnSupplyDropWithinCircle"))
        .String(TEXT("AFortAthenaMutator_Cobalt::SpawnAndRegisterSupplyDrop: Spawned and registered a supply drop"))
        .Start()
        .Find("mov r9, [r9+0x38]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return SpawnSupplyDropWithinCircle(this, Center, Radius, InSupplyDropClass, TraceStartZ, TraceEndZ);
}

AFortAthenaSupplyDrop* AFortAthenaMapInfo::SpawnSupplyDropInSafeZone(int32 GamePhaseIndex, int32 SafeZoneIndex, const UObject* SupplyDropInfo, const TGameValue<FVector>& SafeZoneLocation) const
{
    static auto SupplyDropInfoClass = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSupplyDropInfo"));
    static auto SupplyDropClass = SupplyDropInfoClass->FindPropertyByName(FName(TEXT("SupplyDropClass")));
    static auto GamePhaseData = (UArrayProperty*)SupplyDropInfoClass->FindPropertyByName(FName(TEXT("GamePhaseData")));
    static auto GamePhaseDataStruct = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSupplyDropGamePhaseData"));
    static auto SupplyDropMinPlacementHeight = GamePhaseDataStruct->FindPropertyByName(FName(TEXT("SupplyDropMinPlacementHeight")));
    static auto SupplyDropMaxPlacementHeight = GamePhaseDataStruct->FindPropertyByName(FName(TEXT("SupplyDropMaxPlacementHeight")));

    auto Class = *SupplyDropClass->ContainerPtrToValuePtr<UClass*>(SupplyDropInfo);
    auto& PhaseDataList = *GamePhaseData->ContainerPtrToValuePtr<TArray<uint8>>(SupplyDropInfo);

    if (!GetSafeZoneDefinition() || !Class || !Class->IsChildOf(AFortAthenaSupplyDrop::StaticClass()) || GamePhaseIndex >= PhaseDataList.Num() || SafeZoneIndex >= GetSafeZoneCount())
        return nullptr;

    auto SafeZoneRadius = GetSafeZoneRadius();
    const float Radius = SafeZoneIndex >= 0 && SafeZoneRadius ? SafeZoneRadius->GetValueAtLevel((float)SafeZoneIndex) : 0.0f;
    auto PhaseData = PhaseDataList.GetData() + GamePhaseIndex * GamePhaseData->GetInner()->GetSize();
    const float TraceStartZ = SupplyDropMaxPlacementHeight->ContainerPtrToValuePtr<FScalableFloat>(PhaseData)->GetValueAtLevel(0.0f);
    const float TraceEndZ = SupplyDropMinPlacementHeight->ContainerPtrToValuePtr<FScalableFloat>(PhaseData)->GetValueAtLevel(0.0f);

    return SpawnSupplyDropWithinCircle(SafeZoneLocation, Radius, Class, TraceStartZ, TraceEndZ);
}

UClass* AFortAthenaMapInfo::GetLlamaClass() const
{
    static auto LlamaClass = GetClass()->FindPropertyByName(FName(TEXT("LlamaClass")));

    return LlamaClass ? *LlamaClass->ContainerPtrToValuePtr<UClass*>(this) : nullptr;
}

UClass* AFortAthenaMapInfo::GetAircraftClass() const
{
    static auto AircraftClass = GetClass()->FindPropertyByName(FName(TEXT("AircraftClass")));

    return *AircraftClass->ContainerPtrToValuePtr<UClass*>(this);
}

TArray<FAircraftFlightInfo>& AFortAthenaMapInfo::GetFlightInfos()
{
    static auto FlightInfos = GetClass()->FindPropertyByName(FName(TEXT("FlightInfos")));

    return *FlightInfos->ContainerPtrToValuePtr<TArray<FAircraftFlightInfo>>(this);
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
    static auto Radius = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition")))->FindPropertyByName(FName(TEXT("Radius")))->GetOffset_ForInternal();

    auto Definition = GetSafeZoneDefinition();

    return Definition ? (const FScalableFloat*)(Definition + Radius) : nullptr;
}

const FScalableFloat& AFortAthenaMapInfo::GetSafeZoneStartingRadius() const
{
    static auto SafeZoneStartingRadius = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneStartingRadius")));

    return *SafeZoneStartingRadius->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& AFortAthenaMapInfo::GetSafeZoneStartDelay() const
{
    static auto SafeZoneStartDelay = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneStartDelay")));

    return *SafeZoneStartDelay->ContainerPtrToValuePtr<FScalableFloat>(this);
}

void AFortAthenaMapInfo::GenerateSafeZoneLocations(TArray<uint8>& OutLocations, const FVector& MapCenter, int32 BaseSafeZoneIndex)
{
    static auto ConstructSafeZoneLocationParams = Anchor::Function<void* (*)(AFortAthenaMapInfo*, void*, int32, const TGameValue<FVector>&, const TGameValue<FVector>&, bool, int32)>(TEXT("AFortAthenaMapInfo::ConstructSafeZoneLocationParams"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaMapInfo:ConstructSafeZoneLocationParams"))
        .Get();

    static auto PickSafeZoneLocation = Anchor::Function<TGameValue<FVector>* (*)(AFortAthenaMapInfo*, TGameValue<FVector>*, const void*, const void*)>(TEXT("AFortAthenaMapInfo::PickSafeZoneLocation"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaMapInfo:PickSafeZoneLocation"))
        .Get();

    const int32 SafeZoneCount = GetSafeZoneCount();

    if (SafeZoneCount <= 0)
        return;

    OutLocations.Reset();
    OutLocations.AddUninitialized(SafeZoneCount * TGameLayout<FVector>::Size());
    memset(OutLocations.GetData(), 0, OutLocations.Num());
    TGameLayout<FVector>::Store(OutLocations.GetData(), MapCenter);

    TArray<uint8> BlacklistLocations;

    for (int32 SafeZoneIndex = SafeZoneCount - 1; SafeZoneIndex > BaseSafeZoneIndex; SafeZoneIndex--)
    {
        const bool bFinalPhase = SafeZoneIndex == SafeZoneCount - 1;
        const int32 NextIndex = bFinalPhase ? 0 : SafeZoneIndex + 1;
        const TGameValue<FVector> NextLocation(TGameLayout<FVector>::Load(OutLocations.GetData() + NextIndex * TGameLayout<FVector>::Size()));
        alignas(16) uint8 Params[0x100]{};
        TGameValue<FVector> Location;

        ConstructSafeZoneLocationParams(this, Params, SafeZoneIndex, TGameValue<FVector>(MapCenter), NextLocation, bFinalPhase, BaseSafeZoneIndex);

        auto PickedLocation = PickSafeZoneLocation(this, &Location, Params, &BlacklistLocations);

        if (!PickedLocation)
        {
            OutLocations.Reset();
            return;
        }

        TGameLayout<FVector>::Store(OutLocations.GetData() + SafeZoneIndex * TGameLayout<FVector>::Size(), PickedLocation->Get());
    }
}

int32 AFortAthenaMapInfo::GetSafeZoneCount() const
{
    static auto Count = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition")))->FindPropertyByName(FName(TEXT("Count")))->GetOffset_ForInternal();

    auto Definition = GetSafeZoneDefinition();

    return Definition ? (int32)((const FScalableFloat*)(Definition + Count))->GetValueAtLevel(0.0f) : 0;
}

const uint8* AFortAthenaMapInfo::GetSafeZoneDefinition() const
{
    static auto SafeZoneDefinitions = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneDefinitions")));
    static auto SafeZoneDefinition = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneDefinition")));

    if (!SafeZoneDefinitions)
        return SafeZoneDefinition->ContainerPtrToValuePtr<uint8>(this);

    auto& Definitions = *SafeZoneDefinitions->ContainerPtrToValuePtr<TArray<uint8>>(this);

    return Definitions.Num() ? &Definitions[0] : nullptr;
}
