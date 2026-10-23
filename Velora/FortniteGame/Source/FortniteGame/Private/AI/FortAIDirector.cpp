#include "pch.h"
#include "AI/FortAIDirector.h"
#include "Pawns/FortAIPawn.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"


UScriptStruct* FFortAIPawnLootDropData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIPawnLootDropData"));

    return Struct;
}

float& FFortAIPawnLootDropData::GetLootDropChance()
{
    static auto LootDropChance = StaticStruct()->FindPropertyByName(FName(TEXT("LootDropChance")));

    return *LootDropChance->ContainerPtrToValuePtr<float>(this);
}

FName FFortAIPawnLootDropData::GetWorldItemTierGroup()
{
    static auto WorldItemTierGroup = StaticStruct()->FindPropertyByName(FName(TEXT("WorldItemTierGroup")));

    return WorldItemTierGroup->GetPropertyValue_InContainer<FName>(this);
}

UScriptStruct* FPendingSpawnInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.PendingSpawnInfo"));

    return Struct;
}

FFortAIPawnLootDropData& FPendingSpawnInfo::GetLootDropData()
{
    static auto LootDropData = StaticStruct()->FindPropertyByName(FName(TEXT("LootDropData")));

    return *LootDropData->ContainerPtrToValuePtr<FFortAIPawnLootDropData>(this);
}

template<>
UEnum* StaticEnum<EFortEncounterSpawnLocationPlacementMode>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EFortEncounterSpawnLocationPlacementMode"));

    return Enum;
}

UScriptStruct* FEncounterEnvironmentQueryInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EncounterEnvironmentQueryInfo"));

    return Struct;
}

UEnvQuery*& FEncounterEnvironmentQueryInfo::GetEnvironmentQuery()
{
    static auto EnvironmentQuery = StaticStruct()->FindPropertyByName(FName(TEXT("EnvironmentQuery")));

    return *EnvironmentQuery->ContainerPtrToValuePtr<UEnvQuery*>(this);
}

FFortAIEncounterSpawnGroupCap::FFortAIEncounterSpawnGroupCap()
{
    memset(this, 0, GetTypeSize());
}

UScriptStruct* FFortAIEncounterSpawnGroupCap::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIEncounterSpawnGroupCap"));

    return Struct;
}

int32 FFortAIEncounterSpawnGroupCap::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FCurveTableRowHandle& FFortAIEncounterSpawnGroupCap::GetMinSpawnGroupNumberCap()
{
    static auto MinSpawnGroupNumberCap = StaticStruct()->FindPropertyByName(FName(TEXT("MinSpawnGroupNumberCap")));

    return *MinSpawnGroupNumberCap->ContainerPtrToValuePtr<FCurveTableRowHandle>(this);
}

FCurveTableRowHandle& FFortAIEncounterSpawnGroupCap::GetMaxSpawnGroupNumberCap()
{
    static auto MaxSpawnGroupNumberCap = StaticStruct()->FindPropertyByName(FName(TEXT("MaxSpawnGroupNumberCap")));

    return *MaxSpawnGroupNumberCap->ContainerPtrToValuePtr<FCurveTableRowHandle>(this);
}

FFortAIEncounterSpawnGroupCapsCategory::FFortAIEncounterSpawnGroupCapsCategory()
{
    memset(this, 0, GetTypeSize());
}

UScriptStruct* FFortAIEncounterSpawnGroupCapsCategory::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIEncounterSpawnGroupCapsCategory"));

    return Struct;
}

int32 FFortAIEncounterSpawnGroupCapsCategory::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FGameplayTagQuery& FFortAIEncounterSpawnGroupCapsCategory::GetTagQuery()
{
    static auto TagQuery = StaticStruct()->FindPropertyByName(FName(TEXT("TagQuery")));

    return *TagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this);
}

FCurveTableRowHandle& FFortAIEncounterSpawnGroupCapsCategory::GetInitialSpawnGroupAvailabilityDelaySeconds()
{
    static auto InitialSpawnGroupAvailabilityDelaySeconds = StaticStruct()->FindPropertyByName(FName(TEXT("InitialSpawnGroupAvailabilityDelaySeconds")));

    return *InitialSpawnGroupAvailabilityDelaySeconds->ContainerPtrToValuePtr<FCurveTableRowHandle>(this);
}

FCurveTableRowHandle& FFortAIEncounterSpawnGroupCapsCategory::GetSpawnGroupAvailabilityDelaySeconds()
{
    static auto SpawnGroupAvailabilityDelaySeconds = StaticStruct()->FindPropertyByName(FName(TEXT("SpawnGroupAvailabilityDelaySeconds")));

    return *SpawnGroupAvailabilityDelaySeconds->ContainerPtrToValuePtr<FCurveTableRowHandle>(this);
}

TArray<FFortAIEncounterSpawnGroupCap>& FFortAIEncounterSpawnGroupCapsCategory::GetSpawnGroupCapsPerPlayerCount()
{
    static auto SpawnGroupCapsPerPlayerCount = StaticStruct()->FindPropertyByName(FName(TEXT("SpawnGroupCapsPerPlayerCount")));

    return *SpawnGroupCapsPerPlayerCount->ContainerPtrToValuePtr<TArray<FFortAIEncounterSpawnGroupCap>>(this);
}

float& FFortAIEncounterSpawnGroupCapsCategory::GetInitialSpawnGroupAvailabilityTime()
{
    static auto InitialSpawnGroupAvailabilityTime = StaticStruct()->FindPropertyByName(FName(TEXT("InitialSpawnGroupAvailabilityTime")));

    return *InitialSpawnGroupAvailabilityTime->ContainerPtrToValuePtr<float>(this);
}

FFortAIEncounterSpawnGroupCapsProfile::FFortAIEncounterSpawnGroupCapsProfile()
{
    memset(this, 0, GetTypeSize());
}

UScriptStruct* FFortAIEncounterSpawnGroupCapsProfile::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIEncounterSpawnGroupCapsProfile"));

    return Struct;
}

int32 FFortAIEncounterSpawnGroupCapsProfile::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FGameplayTagContainer& FFortAIEncounterSpawnGroupCapsProfile::GetEncounterTypeTags()
{
    static auto EncounterTypeTags = StaticStruct()->FindPropertyByName(FName(TEXT("EncounterTypeTags")));

    return *EncounterTypeTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

TArray<FFortAIEncounterSpawnGroupCapsCategory>& FFortAIEncounterSpawnGroupCapsProfile::GetPopulationCategories()
{
    static auto PopulationCategories = StaticStruct()->FindPropertyByName(FName(TEXT("PopulationCategories")));

    return *PopulationCategories->ContainerPtrToValuePtr<TArray<FFortAIEncounterSpawnGroupCapsCategory>>(this);
}

FFortAIEncounterSpawnPointsProfile::FFortAIEncounterSpawnPointsProfile()
{
    memset(this, 0, GetTypeSize());
}

UScriptStruct* FFortAIEncounterSpawnPointsProfile::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIEncounterSpawnPointsProfile"));

    return Struct;
}

int32 FFortAIEncounterSpawnPointsProfile::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

FGameplayTagContainer& FFortAIEncounterSpawnPointsProfile::GetEncounterTypeTags()
{
    static auto EncounterTypeTags = StaticStruct()->FindPropertyByName(FName(TEXT("EncounterTypeTags")));

    return *EncounterTypeTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

TArray<FCurveTableRowHandle>& FFortAIEncounterSpawnPointsProfile::GetMaxSpawnPointsPerPlayerCount()
{
    static auto MaxSpawnPointsPerPlayerCount = StaticStruct()->FindPropertyByName(FName(TEXT("MaxSpawnPointsPerPlayerCount")));

    return *MaxSpawnPointsPerPlayerCount->ContainerPtrToValuePtr<TArray<FCurveTableRowHandle>>(this);
}

TArray<FCurveTableRowHandle>& FFortAIEncounterSpawnPointsProfile::GetMinSpawnPointsPerPlayerCount()
{
    static auto MinSpawnPointsPerPlayerCount = StaticStruct()->FindPropertyByName(FName(TEXT("MinSpawnPointsPerPlayerCount")));

    return *MinSpawnPointsPerPlayerCount->ContainerPtrToValuePtr<TArray<FCurveTableRowHandle>>(this);
}

UClass* AFortAIDirector::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIDirector"));

    return Class;
}

AFortAIPawn* AFortAIDirector::SpawnAIPawn(FPendingSpawnInfo& PendingSpawnInfo)
{
    DETOUR(AFortAIDirector, SpawnAIPawn, Anchor::Hook()
        .String(TEXT("AI Director failed to spawn pawn!"))
        .Optional()
        .Start()
        .Inline());

    const float LootDropChance = PendingSpawnInfo.GetLootDropData().GetLootDropChance();
    const FName WorldItemTierGroup = PendingSpawnInfo.GetLootDropData().GetWorldItemTierGroup();

    auto NewPawn = Original(PendingSpawnInfo);

    if (NewPawn)
        NewPawn->SetLootDropData(LootDropChance, WorldItemTierGroup);

    return NewPawn;
}

TArray<FFortAIEncounterSpawnGroupCapsProfile>& AFortAIDirector::GetEncounterSpawnGroupCapSettings()
{
    static auto EncounterSpawnGroupCapSettings = GetClass()->FindPropertyByName(FName(TEXT("EncounterSpawnGroupCapSettings")));

    return *EncounterSpawnGroupCapSettings->ContainerPtrToValuePtr<TArray<FFortAIEncounterSpawnGroupCapsProfile>>(this);
}

TArray<FFortAIEncounterSpawnPointsProfile>& AFortAIDirector::GetEncounterSpawnPointsSettings()
{
    static auto EncounterSpawnPointsSettings = GetClass()->FindPropertyByName(FName(TEXT("EncounterSpawnPointsSettings")));

    return *EncounterSpawnPointsSettings->ContainerPtrToValuePtr<TArray<FFortAIEncounterSpawnPointsProfile>>(this);
}

FEncounterEnvironmentQueryInfo& AFortAIDirector::GetSpawnLocationActorSearchQueries(EFortEncounterSpawnLocationPlacementMode SpawnLocationPlacementMode)
{
    static auto SpawnLocationActorSearchQueries = GetClass()->FindPropertyByName(FName(TEXT("SpawnLocationActorSearchQueries")));

    return *SpawnLocationActorSearchQueries->ContainerPtrToValuePtr<FEncounterEnvironmentQueryInfo>(this, (int32)SpawnLocationPlacementMode);
}

TArray<FDataTableRowHandle>& AFortAIDirector::GetBaseLootDropData()
{
    static auto BaseLootDropData = GetClass()->FindPropertyByName(FName(TEXT("BaseLootDropData")));

    return *BaseLootDropData->ContainerPtrToValuePtr<TArray<FDataTableRowHandle>>(this);
}

TSubclassOf<UFortAIEncounterInfo>& AFortAIDirector::GetDummyDebugEncounter()
{
    static auto DummyDebugEncounter = GetClass()->FindPropertyByName(FName(TEXT("DummyDebugEncounter")));

    return *DummyDebugEncounter->ContainerPtrToValuePtr<TSubclassOf<UFortAIEncounterInfo>>(this);
}

TSubclassOf<UFortAIEncounterInfo>& AFortAIDirector::GetBaseEncounterClass()
{
    static auto BaseEncounterClass = GetClass()->FindPropertyByName(FName(TEXT("BaseEncounterClass")));

    return *BaseEncounterClass->ContainerPtrToValuePtr<TSubclassOf<UFortAIEncounterInfo>>(this);
}
