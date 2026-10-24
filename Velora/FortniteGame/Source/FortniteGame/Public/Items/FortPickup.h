#pragma once

#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"

class AFortPawn;
class AFortPickup;
class AFortPlayerController;
class IFortInventoryOwnerInterface;
class UScriptStruct;
class UWorld;
struct FFortItemEntry;
struct FRotator;

enum class EFortPickupTossState : uint8
{
    NotTossed,
    InProgress,
    AtRest,
};

enum class EFortPickupSourceTypeFlag : uint8
{
    Other = 0,
    Player = 1,
    Destruction = 2,
    Container = 4,
    AI = 8,
    Tossed = 16,
    FloorLoot = 32,
};

enum class EFortPickupSpawnSource : uint8
{
    Unset = 0,
    PlayerElimination = 1,
    Chest = 2,
    SupplyDrop = 3,
    AmmoBox = 4,
};

struct FFortPickupLocationData
{
    static UScriptStruct* StaticStruct();

    AFortPawn* GetPickupTarget() const;
};

struct FFortPickupRequestInfo
{
    static UScriptStruct* StaticStruct();

    FGuid GetSwapWithItem() const;

    float GetFlyTime() const;

    FVector GetDirection() const;

    bool GetPlayPickupSound() const;

    bool GetUseRequestedSwap() const;
};

struct FortPickupCreationData
{
    UWorld* World;
    const FFortItemEntry* ItemEntry;
    const TGameValue<FVector>* SpawnLocation;
    const TGameValue<FRotator>* SpawnRotation;
    AFortPlayerController* OptionalOwnerPC;
    UClass* OverrideClass;
    AActor* Owner;
    bool bRandomRotation;
    uint32 SourceTypeFlags;
    EFortPickupSpawnSource PickupSpawnSource;
};

class AFortPickup : public AActor
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, AFortPickup*, float>& GetNotifyPickupCullDistanceChange();

    static AFortPickup* CreateFromData(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& SpawnLocation, const FRotator& SpawnRotation, AFortPlayerController* OptionalOwnerPC = nullptr, UClass* OverrideClass = nullptr, AActor* Owner = nullptr, EFortPickupSourceTypeFlag SourceTypeFlags = EFortPickupSourceTypeFlag::Other, bool bRandomRotation = true, bool bPickupOnlyRelevantToOwner = false, EFortPickupSpawnSource PickupSpawnSource = EFortPickupSpawnSource::Unset);

    void TossPickup(const FVector& FinalLocation, AFortPawn* ItemOwner, int32 OverrideMaxStackCount, bool bToss, bool bShouldCombinePickupsWhenTossCompletes = true, EFortPickupSourceTypeFlag InPickupSourceTypeFlags = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource InPickupSpawnSource = EFortPickupSpawnSource::Unset);

    void GivePickupTo(IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup);

    static void (AFortPickup::*GameGivePickupTo)(IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup);

    void SetSwapItemGuid(const FGuid& SwapItemGuid);

    void GivePickupItemsToInventoryOwner(IFortInventoryOwnerInterface* InventoryOwner);

    FFortItemEntry& GetPrimaryPickupItemEntry();

    TArray<FFortItemEntry>& GetMultiItemPickupEntries();

    FFortPickupLocationData& GetPickupLocationData();

    bool& GetTossedFromContainer();

    void SetPawnWhoDroppedPickup(AFortPawn* InPawnWhoDroppedPickup);

    void SetPickupTarget(AFortPawn* InPickupTarget, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound);

    float GetFlyTime() const;
};
