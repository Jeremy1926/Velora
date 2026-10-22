#include "pch.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AFortInventory::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventory"));

    return Class;
}

EFortInventoryType& AFortInventory::GetInventoryType()
{
    static auto InventoryType = GetClass()->FindPropertyByName(FName(TEXT("InventoryType")));

    return *InventoryType->ContainerPtrToValuePtr<EFortInventoryType>(this);
}

FFortItemList& AFortInventory::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<FFortItemList>(this);
}

void AFortInventory::HandleInventoryLocalUpdate()
{
    static auto HandleInventoryLocalUpdate = Anchor::Function<void (*)(AFortInventory*)>(TEXT("AFortInventory::HandleInventoryLocalUpdate"))
        .Exec(TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    HandleInventoryLocalUpdate(this);
}

int32 AFortInventory::GetOverflowFromAddingItem(const FFortItemEntry& ItemEntry) const
{
    static auto GetOverflowFromAddingItemForOwner = Anchor::Optional<int32 (*)(const FFortItemEntry&, void*, uint8, bool)>(TEXT("AFortInventory::GetOverflowFromAddingItem"))
        .String(TEXT("SwapEquippedText"))
        .Find("mov rdx, rax; call 0x*; test eax, eax")
        .Call()
        .Get();

    if (GetOverflowFromAddingItemForOwner)
    {
        static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));
        static auto InventoryType = GetClass()->FindPropertyByName(FName(TEXT("InventoryType")));
        auto Owner = GetOwner();

        return GetOverflowFromAddingItemForOwner(ItemEntry, Owner ? Owner->GetInterfaceAddress(FortInventoryOwnerInterface) : nullptr, *InventoryType->ContainerPtrToValuePtr<uint8>(this), false);
    }

    static auto GetOverflowFromAddingItem = Anchor::Function<int32 (*)(const AFortInventory*, const FFortItemEntry&)>(TEXT("AFortInventory::GetOverflowFromAddingItem"))
        .String(TEXT("SwapEquippedText"))
        .Find("mov rcx, *; movzx *, al; call 0x*; test eax, eax")
        .Call()
        .Or()
        .String(TEXT("SwapEquippedText"))
        .FindEither("mov rcx, rax; call 0x*", "mov rcx, *; call 0x*; test eax, eax")
        .Call()
        .Get();

    return GetOverflowFromAddingItem(this, ItemEntry);
}

void AFortInventory::SetItemRequiresUpdate(FFortItemEntry& ItemEntry)
{
    static auto SetItemRequiresUpdate = Anchor::Function<void (*)(AFortInventory*, FFortItemEntry&)>(TEXT("AFortInventory::SetItemRequiresUpdate"))
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov rcx, rax; mov rdx, *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"))
        .Find("mov edx, ebp; call 0x*")
        .Call()
        .Find("mov rcx, rax; mov rdx, *; call 0x*")
        .Call()
        .Get();

    SetItemRequiresUpdate(this, ItemEntry);
}

FFortItemEntryStateValue::FFortItemEntryStateValue(EFortItemEntryState::Type InStateType, int32 InIntValue)
{
    memset(this, 0, GetTypeSize());

    GetIntValue() = InIntValue;
    GetStateType() = InStateType;
}

UScriptStruct* FFortItemEntryStateValue::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntryStateValue"));

    return Struct;
}

int32 FFortItemEntryStateValue::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32& FFortItemEntryStateValue::GetIntValue()
{
    static auto IntValue = StaticStruct()->FindPropertyByName(FName(TEXT("IntValue")));

    return *IntValue->ContainerPtrToValuePtr<int32>(this);
}

TEnumAsByte<EFortItemEntryState::Type>& FFortItemEntryStateValue::GetStateType()
{
    static auto StateType = StaticStruct()->FindPropertyByName(FName(TEXT("StateType")));

    return *StateType->ContainerPtrToValuePtr<TEnumAsByte<EFortItemEntryState::Type>>(this);
}

FFortItemEntry::FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel)
{
    static auto Constructor = Anchor::Function<void (*)(FFortItemEntry*, UFortItemDefinition*, int32, int32, const TArray<UObject*>*)>(TEXT("FFortItemEntry::FFortItemEntry"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    Constructor(this, InItemDefinition, InCount, InLevel, nullptr);
}

FFortItemEntry::FFortItemEntry(const FFortItemEntry& Other)
{
    static auto Constructor = Anchor::Optional<void (*)(FFortItemEntry*, const FFortItemEntry&)>(TEXT("FFortItemEntry::FFortItemEntry(const FFortItemEntry&)"))
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("lea r8d, [r9+0x1]; call 0x*; mov rdx, rax")
        .Call(2)
        .Optional()
        .Get();

    if (Constructor)
    {
        Constructor(this, Other);

        return;
    }

    StaticStruct()->InitializeStruct(this);
    StaticStruct()->CopyScriptStruct(this, &Other);
}

FFortItemEntry::~FFortItemEntry()
{
    static auto Destructor = Anchor::Function<void (*)(FFortItemEntry*)>(TEXT("FFortItemEntry::~FFortItemEntry"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("add rcx, [*]")
        .Find("lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    Destructor(this);
}

UScriptStruct* FFortItemEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemEntry"));

    return Struct;
}

int32 FFortItemEntry::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32& FFortItemEntry::GetCount()
{
    static auto Count = StaticStruct()->FindPropertyByName(FName(TEXT("Count")));

    return *Count->ContainerPtrToValuePtr<int32>(this);
}

UFortItemDefinition*& FFortItemEntry::GetItemDefinition()
{
    static auto ItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("ItemDefinition")));

    return *ItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);
}

float& FFortItemEntry::GetDurability()
{
    static auto Durability = StaticStruct()->FindPropertyByName(FName(TEXT("Durability")));

    return *Durability->ContainerPtrToValuePtr<float>(this);
}

int32& FFortItemEntry::GetLevel()
{
    static auto Level = StaticStruct()->FindPropertyByName(FName(TEXT("Level")));

    return *Level->ContainerPtrToValuePtr<int32>(this);
}

int32& FFortItemEntry::GetLoadedAmmo()
{
    static auto LoadedAmmo = StaticStruct()->FindPropertyByName(FName(TEXT("LoadedAmmo")));

    return *LoadedAmmo->ContainerPtrToValuePtr<int32>(this);
}

FGuid& FFortItemEntry::GetItemGuid()
{
    static auto ItemGuidProperty = StaticStruct()->FindPropertyByName(FName(TEXT("ItemGuid")));

    auto& ItemGuid = *ItemGuidProperty->ContainerPtrToValuePtr<FGuid>(this);

    if (!ItemGuid.IsValid())
        ItemGuid = FGuid::NewGuid();

    return ItemGuid;
}

bool& FFortItemEntry::GetIsReplicatedCopy()
{
    static auto bIsReplicatedCopy = StaticStruct()->FindPropertyByName(FName(TEXT("bIsReplicatedCopy")));

    return *bIsReplicatedCopy->ContainerPtrToValuePtr<bool>(this);
}

bool& FFortItemEntry::GetIsDirty()
{
    static auto bIsDirty = StaticStruct()->FindPropertyByName(FName(TEXT("bIsDirty")));

    return *bIsDirty->ContainerPtrToValuePtr<bool>(this);
}

TArray<FFortItemEntryStateValue>& FFortItemEntry::GetStateValues()
{
    static auto StateValues = StaticStruct()->FindPropertyByName(FName(TEXT("StateValues")));

    return *StateValues->ContainerPtrToValuePtr<TArray<FFortItemEntryStateValue>>(this);
}

TWeakObjectPtr<AFortInventory>& FFortItemEntry::GetParentInventory()
{
    static auto ParentInventory = StaticStruct()->FindPropertyByName(FName(TEXT("ParentInventory")));

    return *ParentInventory->ContainerPtrToValuePtr<TWeakObjectPtr<AFortInventory>>(this);
}

void FFortItemEntry::SetCount(int32 NewCount)
{
    NewCount = FMath::Max(NewCount, 0);

    if (GetCount() != NewCount)
    {
        GetCount() = NewCount;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

void FFortItemEntry::SetDurability(float NewDurability)
{
    if (GetDurability() != NewDurability)
    {
        GetDurability() = NewDurability;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

void FFortItemEntry::SetLoadedAmmo(int32 NewLoadedAmmo)
{
    if (GetLoadedAmmo() != NewLoadedAmmo)
    {
        GetLoadedAmmo() = NewLoadedAmmo;

        if (GetParentInventory().IsValid())
        {
            GetParentInventory().Get()->SetItemRequiresUpdate(*this);
        }

        GetIsDirty() = true;
    }
}

bool FFortItemEntry::GetStateValue(EFortItemEntryState::Type StateType, int32& OutIntValue)
{
    for (auto& StateValue : GetStateValues())
    {
        if (StateValue.GetStateType() == StateType)
        {
            OutIntValue = StateValue.GetIntValue();
            return true;
        }
    }

    return false;
}

void FFortItemEntry::SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue)
{
    auto& StateValues = GetStateValues();
    int32 StateValueIndex = 0;

    while (StateValueIndex < StateValues.Num() && StateValues[StateValueIndex].GetStateType() != StateType)
    {
        StateValueIndex++;
    }

    if (StateValueIndex < StateValues.Num())
    {
        StateValues[StateValueIndex].GetIntValue() = IntValue;
    }
    else
    {
        new(StateValues) FFortItemEntryStateValue(StateType, IntValue);
    }

    if (GetParentInventory().IsValid())
    {
        GetParentInventory().Get()->SetItemRequiresUpdate(*this);
    }

    GetIsDirty() = true;
}

void FFortItemEntry::RemoveStateValue(EFortItemEntryState::Type StateType)
{
    auto& StateValues = GetStateValues();

    for (int32 StateValueIndex = 0; StateValueIndex < StateValues.Num(); StateValueIndex++)
    {
        if (StateValues[StateValueIndex].GetStateType() == StateType)
        {
            StateValues.RemoveAt(StateValueIndex);

            if (GetParentInventory().IsValid())
            {
                GetParentInventory().Get()->SetItemRequiresUpdate(*this);
            }

            GetIsDirty() = true;

            return;
        }
    }
}

UScriptStruct* FFortItemList::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemList"));

    return Struct;
}

TArray<FFortItemEntry>& FFortItemList::GetReplicatedEntries()
{
    static auto ReplicatedEntries = StaticStruct()->FindPropertyByName(FName(TEXT("ReplicatedEntries")));

    return *ReplicatedEntries->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

TArray<UFortWorldItem*>& FFortItemList::GetItemInstances()
{
    static auto ItemInstances = StaticStruct()->FindPropertyByName(FName(TEXT("ItemInstances")));

    return *ItemInstances->ContainerPtrToValuePtr<TArray<UFortWorldItem*>>(this);
}
