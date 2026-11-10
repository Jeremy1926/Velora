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
#include "UObject/Class.h"

class AFortInventory;
class UFortItemDefinition;
class UFortWorldItem;
struct FGameplayTag;
struct FFortWeaponModSlot;
struct FFortSavedWeaponModSlot;

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
    FFortItemEntryStateValue(EFortItemEntryState::Type InStateType, int32 InIntValue);

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    int32& GetIntValue();

    TEnumAsByte<EFortItemEntryState::Type>& GetStateType();
};

template<>
class TArray<FFortItemEntryStateValue, FDefaultAllocator> : public TRuntimeSizedArray<FFortItemEntryStateValue>
{
};

struct FFortItemEntryStateValueConstObject
{
    FFortItemEntryStateValueConstObject(const FGameplayTag& InStateType, const UObject* InObjectData);

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FGameplayTag GetStateType() const;

    TWeakObjectPtr<UObject>& GetObjectData();
};

template<>
class TArray<FFortItemEntryStateValueConstObject, FDefaultAllocator> : public TRuntimeSizedArray<FFortItemEntryStateValueConstObject>
{
};

struct alignas(8) FFortItemEntry : public FFastArraySerializerItem
{
    FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel);

    FFortItemEntry(const FFortItemEntry& Other);

    FFortItemEntry& operator=(const FFortItemEntry&) = delete;

    ~FFortItemEntry();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    int32& GetCount();

    UFortItemDefinition*& GetItemDefinition();

    float& GetDurability();

    int32& GetLevel();

    int32& GetLoadedAmmo();

    FGuid& GetItemGuid();

    bool& GetIsReplicatedCopy();

    bool& GetIsDirty();

    TArray<FFortItemEntryStateValue>& GetStateValues();

    TWeakObjectPtr<AFortInventory>& GetParentInventory();

    void SetCount(int32 NewCount);

    void SetDurability(float NewDurability);

    void SetLoadedAmmo(int32 NewLoadedAmmo);

    bool GetStateValue(EFortItemEntryState::Type StateType, int32& OutIntValue);

    void SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue);

    void RemoveStateValue(EFortItemEntryState::Type StateType);

    TArray<FFortItemEntryStateValueConstObject>& GetStateValuesConstObject();

    const UObject* GetStateConstObject(const FGameplayTag& StateType);

    void SetStateConstObject(const FGameplayTag& StateType, const UObject* ObjectData);

    void RemoveStateConstObject(const FGameplayTag& StateType);

    TArray<FFortWeaponModSlot>& GetWeaponModSlots();

    TArray<FFortSavedWeaponModSlot>& GetSavedWeaponModSlots();

    void AppendWeaponModSlot(const FFortWeaponModSlot& WeaponModSlot);

    void SetWeaponModSlots(const TArray<FFortWeaponModSlot>& InWeaponModSlots);

private:
    uint8 Data[0x1F4];
};

template<>
class TArray<FFortItemEntry, FDefaultAllocator> : public TRuntimeSizedArray<FFortItemEntry>
{
};

struct FFortItemList : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    TArray<FFortItemEntry>& GetReplicatedEntries();

    TArray<UFortWorldItem*>& GetItemInstances();
};

enum class EFortInventoryType : uint8
{
    World,
    Account,
    Outpost,
    MAX
};

class AFortInventory : public AActor
{
public:
    static UClass* StaticClass();

    EFortInventoryType& GetInventoryType();

    FFortItemList& GetInventory();

    void HandleInventoryLocalUpdate();

    void SetItemRequiresUpdate(FFortItemEntry& ItemEntry);

    int32 GetOverflowFromAddingItem(const FFortItemEntry& ItemEntry) const;
};
