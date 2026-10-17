#pragma once

#include "GameFramework/Actor.h"
#include "Engine/NetSerialization.h"
#include "Containers/EnumAsByte.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Guid.h"
#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"
#include "GameplayAbilitySpec.h"

class AFortInventory;
class FUniqueNetId;
class UFortAlterationItemDefinition;
class UFortHeroType;
class UFortItemDefinition;
class UFortWorldItem;

namespace EFortItemEntryState
{
    enum Type
    {
        NoneState,
        NewItemCount,
        ShouldShowItemToast,
        DurabilityInitialized,
        DoNotShowSpawnParticles,
        FromRecoveredBackpack,
        FromGift,
        PendingUpgradeCriteriaProgress,
        OwnerBuildingHandle,
        FromDroppedPickup,
        CraftAndSlotTarget,
    };
}

struct FItemAndCount
{
    int32 Count;
    UFortItemDefinition* Item;
};

struct FFortItemEntryStateValue
{
    int32 IntValue;
    FName NameValue;
    TEnumAsByte<EFortItemEntryState::Type> StateType;
};

struct FFortGiftingInfo
{
    FFortGiftingInfo()
        : HeroType(nullptr)
    {
    }

    FString PlayerName;
    UFortHeroType* HeroType;
    TSharedPtr<const FUniqueNetId> PlayerId;
};

struct FFortItemEntry : public FFastArraySerializerItem
{
    FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel)
        : Count(FMath::Max(InCount, 0))
        , ItemDefinition(InItemDefinition)
        , Durability(1.0f)
        , Level(FMath::Max(InLevel, 0))
        , LoadedAmmo(0)
        , ItemGuid(FGuid::NewGuid())
        , inventory_overflow_date(false)
        , bIsReplicatedCopy(false)
        , bIsDirty(false)
    {
    }

    int32 Count;
    UFortItemDefinition* ItemDefinition;
    float Durability;
    int32 Level;
    int32 LoadedAmmo;
    TArray<UFortAlterationItemDefinition*> AlterationDefinitions;
    FString ItemSource;
    FGuid ItemGuid;
    bool inventory_overflow_date;
    bool bIsReplicatedCopy;
    bool bIsDirty;
    FFortGiftingInfo GiftingInfo;
    TArray<FFortItemEntryStateValue> StateValues;
    TWeakObjectPtr<AFortInventory> ParentInventory;
    FGameplayAbilitySpecHandle GameplayAbilitySpecHandle;

    void SetCount(int32 NewCount);

    void SetDurability(float NewDurability);

    void SetLoadedAmmo(int32 NewLoadedAmmo);

    void SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue);
};

struct FFortItemList : public FFastArraySerializer
{
    TArray<FFortItemEntry> ReplicatedEntries;
    TMap<FGuid, FFortItemEntry*> ItemEntryMap;
    TArray<UFortWorldItem*> ItemInstances;
    TMap<FGuid, UFortWorldItem*> ItemInstanceMap;
};

class AFortInventory : public AActor
{
public:
    FFortItemList& GetInventory();

    void HandleInventoryLocalUpdate();

    void SetItemRequiresUpdate(FFortItemEntry& ItemEntry);
};
