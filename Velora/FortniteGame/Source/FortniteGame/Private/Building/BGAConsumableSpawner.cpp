#include "pch.h"
#include "Building/BGAConsumableSpawner.h"
#include "Building/BuildingGameplayActorConsumable.h"
#include "Items/BGAConsumableWrapperItemDefinition.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"

struct FPendingConsumableSpawn
{
    TWeakObjectPtr<ABGAConsumableSpawner> Spawner;
    TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper> QueryInstance;
    int32 TicksWaited;
};

static TArray<FPendingConsumableSpawn> PendingSpawns;

UClass* ABGAConsumableSpawner::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BGAConsumableSpawner"));

    return Class;
}

void ABGAConsumableSpawner::SpawnConsumables()
{
    if (!HasAuthority())
        return;

    auto QueryInstance = GetQueryTemplate() ? UEnvQueryManager::RunEQSQuery(this, GetQueryTemplate(), this, EEnvQueryRunMode::AllMatching, nullptr) : nullptr;

    if (!QueryInstance)
    {
        SpawnConsumablesAtLocations(TArray<FVector>());
        return;
    }

    PendingSpawns.Add(FPendingConsumableSpawn{ this, QueryInstance, 0 });
}

void ABGAConsumableSpawner::SpawnConsumablesAtLocations(TArray<FVector> Locations)
{
    TArray<FFortItemEntry> LootToDrop;
    UFortLootPackage::PickLootDrops(LootToDrop, -1, GetSpawnLootTierGroup(), GetStaticGameplayTags(), true, false);

    for (auto& ItemEntry : LootToDrop)
    {
        auto WrapperItemDefinition = Cast<UBGAConsumableWrapperItemDefinition>(ItemEntry.GetItemDefinition());
        auto ConsumableClass = WrapperItemDefinition ? WrapperItemDefinition->GetConsumableClass().LoadSynchronous() : nullptr;

        if (!ConsumableClass)
            continue;

        auto ConsumableDefault = (ABuildingGameplayActorConsumable*)ConsumableClass->GetDefaultObject();

        for (int32 Index = 0; Index < ItemEntry.GetCount(); Index++)
        {
            FVector SpawnLocation = GetActorLocation();

            if (Locations.Num())
            {
                const int32 LocationIndex = FMath::Min(FMath::TruncToInt(FMath::FRand() * Locations.Num()), Locations.Num() - 1);
                SpawnLocation = Locations[LocationIndex];
                Locations.RemoveAt(LocationIndex);
            }

            const FRotator SpawnRotation{ 0.0f, ConsumableDefault && ConsumableDefault->GetSpawnerCalculateRandomRotation() ? FMath::FRand() * 360.0f : 0.0f, 0.0f };

            GetWorld()->SpawnActor(ConsumableClass, &SpawnLocation, &SpawnRotation);
        }
    }
}

void ABGAConsumableSpawner::TickPendingSpawns()
{
    for (int32 Index = PendingSpawns.Num() - 1; Index >= 0; Index--)
    {
        auto& Pending = PendingSpawns[Index];
        auto Spawner = Pending.Spawner.Get();
        auto QueryInstance = Pending.QueryInstance.Get();

        if (!Spawner)
        {
            PendingSpawns.RemoveAt(Index);
            continue;
        }

        auto Locations = QueryInstance ? QueryInstance->GetResultsAsLocations() : TArray<FVector>();

        if (!Locations.Num() && QueryInstance && ++Pending.TicksWaited < 300)
            continue;

        PendingSpawns.RemoveAt(Index);
        Spawner->SpawnConsumablesAtLocations(Locations);
    }
}

FName ABGAConsumableSpawner::GetSpawnLootTierGroup() const
{
    static auto SpawnLootTierGroup = GetClass()->FindPropertyByName(FName(TEXT("SpawnLootTierGroup")));

    return *SpawnLootTierGroup->ContainerPtrToValuePtr<FName>(this);
}

UEnvQuery* ABGAConsumableSpawner::GetQueryTemplate() const
{
    static auto QueryTemplate = GetClass()->FindPropertyByName(FName(TEXT("QueryTemplate")));

    return *QueryTemplate->ContainerPtrToValuePtr<UEnvQuery*>(this);
}
