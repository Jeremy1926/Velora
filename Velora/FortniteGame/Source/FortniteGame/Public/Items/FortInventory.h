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
#include "UObject/UnrealType.h"

class AFortInventory;
class UBoolProperty;
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

template<>
UEnum* StaticEnum<EFortItemEntryState::Type>();

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

class FBoolPropertyReference
{
public:
    FBoolPropertyReference(const UBoolProperty* InProperty, void* InContainer, const UScriptStruct* InItemComponentData = nullptr);

    operator bool() const;

    void operator=(bool NewValue);

private:
    const UBoolProperty* Property;
    void* Container;
    const UScriptStruct* ItemComponentData;
};

template<typename ValueType>
class TPropertyReference
{
public:
    TPropertyReference(const UProperty* InProperty, void* InContainer, const UScriptStruct* InItemComponentData = nullptr)
        : Property(InProperty)
        , Container(InContainer)
        , ItemComponentData(InItemComponentData)
    {
    }

    operator ValueType() const;

    void operator=(ValueType NewValue);

    void operator=(const TPropertyReference& Other)
    {
        *this = (ValueType)Other;
    }

private:
    const UProperty* Property;
    void* Container;
    const UScriptStruct* ItemComponentData;
};

struct alignas(8) FFortItemEntry : public FFastArraySerializerItem
{
    FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel);

    FFortItemEntry(const FFortItemEntry& Other);

    FFortItemEntry& operator=(const FFortItemEntry&) = delete;

    ~FFortItemEntry();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    const uint8* GetItemComponentData(const UScriptStruct* ScriptStruct);

    void SetItemComponentData(const UScriptStruct* ScriptStruct, const void* StructMemory);

    int32 GetCount();

    UFortItemDefinition*& GetItemDefinition();

    TPropertyReference<float> GetDurability();

    TPropertyReference<int32> GetLevel();

    TPropertyReference<int32> GetLoadedAmmo();

    TPropertyReference<int32> GetPhantomReserveAmmo();

    FGuid& GetItemGuid();

    FBoolPropertyReference GetIsReplicatedCopy();

    FBoolPropertyReference GetIsDirty();

    const TArray<FFortItemEntryStateValue>& GetStateValues();

    TWeakObjectPtr<AFortInventory>& GetParentInventory();

    void SetCount(int32 NewCount);

    void SetDurability(float NewDurability);

    void SetLoadedAmmo(int32 NewLoadedAmmo);

    void SetPhantomReserveAmmo(int32 NewPhantomReserveAmmo);

    void GiveFullAmmo(UObject* WorldContextObject);

    bool GetStateValue(EFortItemEntryState::Type StateType, int32& OutIntValue);

    void SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue);

    void RemoveStateValue(EFortItemEntryState::Type StateType);

    const TArray<FFortItemEntryStateValueConstObject>& GetStateValuesConstObject();

    const UObject* GetStateConstObject(const FGameplayTag& StateType);

    void SetStateConstObject(const FGameplayTag& StateType, const UObject* ObjectData);

    void RemoveStateConstObject(const FGameplayTag& StateType);

    const TArray<FFortWeaponModSlot>& GetWeaponModSlots();

    const TArray<FFortSavedWeaponModSlot>& GetSavedWeaponModSlots();

    void AppendWeaponModSlot(const FFortWeaponModSlot& WeaponModSlot);

    void SetWeaponModSlots(const TArray<FFortWeaponModSlot>& InWeaponModSlots);

    void CopyWeaponModStateFrom(FFortItemEntry& Other);

private:
    uint8 Data[0x1F4];
};

template<typename ValueType>
TPropertyReference<ValueType>::operator ValueType() const
{
    return *Property->ContainerPtrToValuePtr<ValueType>(ItemComponentData ? ((FFortItemEntry*)Container)->GetItemComponentData(ItemComponentData) : Container);
}

template<typename ValueType>
void TPropertyReference<ValueType>::operator=(ValueType NewValue)
{
    if (!ItemComponentData)
    {
        *Property->ContainerPtrToValuePtr<ValueType>(Container) = NewValue;
        return;
    }

    auto ItemEntry = (FFortItemEntry*)Container;
    auto CurrentData = ItemEntry->GetItemComponentData(ItemComponentData);

    if (*Property->ContainerPtrToValuePtr<ValueType>(CurrentData) == NewValue)
        return;

    TArray<uint8> NewData;
    NewData.AddUninitialized(ItemComponentData->GetPropertiesSize());

    ItemComponentData->InitializeStruct(NewData.GetData());
    ItemComponentData->CopyScriptStruct(NewData.GetData(), CurrentData);
    *Property->ContainerPtrToValuePtr<ValueType>(NewData.GetData()) = NewValue;

    ItemEntry->SetItemComponentData(ItemComponentData, NewData.GetData());
    ItemComponentData->DestroyStruct(NewData.GetData());
}

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

    void HandleItemsUpdated(const TArray<UFortWorldItem*>& PreviousItemInstances);

    void SetItemRequiresUpdate(FFortItemEntry& ItemEntry);

    int32 GetOverflowFromAddingItem(const FFortItemEntry& ItemEntry) const;
};
