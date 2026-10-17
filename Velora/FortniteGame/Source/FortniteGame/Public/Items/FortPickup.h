#pragma once

#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"

class AFortPawn;
class AFortPickup;
class AFortPlayerController;
class IFortInventoryOwnerInterface;
class UWorld;
struct FFortItemEntry;
struct FRotator;

enum class EFortPickupTossState : uint8
{
    NotTossed,
    InProgress,
    AtRest,
};

struct FFortPickupLocationData
{
    AFortPawn* PickupTarget;
    AFortPickup* CombineTarget;
    AFortPawn* ItemOwner;
    FVector_NetQuantize10 LootInitialPosition;
    FVector_NetQuantize10 LootFinalPosition;
    float FlyTime;
    FVector_NetQuantizeNormal StartDirection;
    FVector_NetQuantize10 FinalTossRestLocation;
    EFortPickupTossState TossState;
    FGuid PickupGuid;
};

class AFortPickup : public AActor
{
public:
    static UClass* StaticClass();

    static AFortPickup* CreateFromData(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& SpawnLocation, const FRotator& SpawnRotation, AFortPlayerController* OptionalOwnerPC = nullptr, UClass* OverrideClass = nullptr);

    void TossPickup(const FVector& FinalLocation, AFortPawn* ItemOwner, int32 OverrideMaxStackCount, bool bToss);

    void GivePickupTo(IFortInventoryOwnerInterface* InventoryOwner);

    void GivePickupItemsToInventoryOwner(IFortInventoryOwnerInterface* InventoryOwner);

    FFortItemEntry& GetPrimaryPickupItemEntry();

    TArray<FFortItemEntry>& GetMultiItemPickupEntries();

    FFortPickupLocationData& GetPickupLocationData();

    bool& GetTossedFromContainer();
};
