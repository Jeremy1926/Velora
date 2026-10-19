#include "pch.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

FFortItemList& AFortInventory::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<FFortItemList>(this);
}

void AFortInventory::HandleInventoryLocalUpdate()
{
    static auto HandleInventoryLocalUpdate = Scanner::Function<void (*)(AFortInventory*)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate")).Find("jmp 0x*").Target(TEXT("AFortInventory::HandleInventoryLocalUpdate")));

    HandleInventoryLocalUpdate(this);
}

int32 AFortInventory::GetOverflowFromAddingItem(const FFortItemEntry& ItemEntry) const
{
    static auto GetOverflowFromAddingItem = Scanner::Function<int32 (*)(const AFortInventory*, const FFortItemEntry&)>(Scanner::String(TEXT("SwapEquippedText")).Find("mov rcx, rax; call 0x*").Call().Get(TEXT("AFortInventory::GetOverflowFromAddingItem")));

    return GetOverflowFromAddingItem(this, ItemEntry);
}

void AFortInventory::SetItemRequiresUpdate(FFortItemEntry& ItemEntry)
{
    static auto SetItemRequiresUpdate = []
    {
        auto UpdateQuantity = Scanner::String(TEXT("UFortWorldItem::UpdateQuantity: Got mcp request to change quantity on item %s*"));
        auto Call = UpdateQuantity.Find("mov rcx, rax; mov rdx, *; call 0x*");

        return Scanner::Function<void (*)(AFortInventory*, FFortItemEntry&)>((Call.Get() ? Call : UpdateQuantity.Find("mov edx, ebp; call 0x*").Call().Find("mov rcx, rax; mov rdx, *; call 0x*")).Call().Get(TEXT("AFortInventory::SetItemRequiresUpdate")));
    }();

    SetItemRequiresUpdate(this, ItemEntry);
}

FFortItemEntry::FFortItemEntry(UFortItemDefinition* InItemDefinition, int32 InCount, int32 InLevel)
{
    static auto Constructor = Scanner::Function<void (*)(FFortItemEntry*, UFortItemDefinition*, int32, int32, const TArray<UObject*>*)>(Scanner::String(TEXT("%s HostUpgradeLevel: %i")).Start().Find("mov rdx, rax; call 0x*").Find("call 0x*").Target(TEXT("FFortItemEntry::FFortItemEntry")));

    Constructor(this, InItemDefinition, InCount, InLevel, nullptr);
}

FFortItemEntry::~FFortItemEntry()
{
    static auto Destructor = Scanner::Function<void (*)(FFortItemEntry*)>(Scanner::String(TEXT("%s HostUpgradeLevel: %i")).Start().Find("mov rdx, rax; call 0x*").Find("add rcx, [*]").Find("lea rcx, [*]; call 0x*").Find("call 0x*").Target(TEXT("FFortItemEntry::~FFortItemEntry")));

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
    static auto ItemGuid = StaticStruct()->FindPropertyByName(FName(TEXT("ItemGuid")));

    return *ItemGuid->ContainerPtrToValuePtr<FGuid>(this);
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
        if (StateValue.StateType == StateType)
        {
            OutIntValue = StateValue.IntValue;
            return true;
        }
    }

    return false;
}

void FFortItemEntry::SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue)
{
    auto& StateValues = GetStateValues();
    int32 StateValueIndex = 0;

    while (StateValueIndex < StateValues.Num() && StateValues[StateValueIndex].StateType != StateType)
    {
        StateValueIndex++;
    }

    if (StateValueIndex < StateValues.Num())
    {
        StateValues[StateValueIndex].IntValue = IntValue;
    }
    else
    {
        StateValues.Add(FFortItemEntryStateValue{ IntValue, FName(NAME_None), StateType });
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
        if (StateValues[StateValueIndex].StateType == StateType)
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
