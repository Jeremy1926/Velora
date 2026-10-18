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
    static auto HandleInventoryLocalUpdate = (void (*)(AFortInventory*))Scanner::Exec(TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate")).Find("jmp 0x*").Target(TEXT("AFortInventory::HandleInventoryLocalUpdate"));

    HandleInventoryLocalUpdate(this);
}

void AFortInventory::SetItemRequiresUpdate(FFortItemEntry& ItemEntry)
{
    static auto SetItemRequiresUpdate = (void (*)(AFortInventory*, FFortItemEntry&))Scanner::Native(TEXT("/Script/FortniteGame.FortOutpostFunctionLibrary:StorageVaultStoreItem")).Call(3).Find("ret").Last("call 0x*").Call().Get(TEXT("AFortInventory::SetItemRequiresUpdate"));

    SetItemRequiresUpdate(this, ItemEntry);
}

void FFortItemEntry::SetCount(int32 NewCount)
{
    NewCount = FMath::Max(NewCount, 0);

    if (Count != NewCount)
    {
        Count = NewCount;

        if (ParentInventory.IsValid())
        {
            ParentInventory.Get()->SetItemRequiresUpdate(*this);
        }

        bIsDirty = true;
    }
}

void FFortItemEntry::SetDurability(float NewDurability)
{
    if (Durability != NewDurability)
    {
        Durability = NewDurability;

        if (ParentInventory.IsValid())
        {
            ParentInventory.Get()->SetItemRequiresUpdate(*this);
        }

        bIsDirty = true;
    }
}

void FFortItemEntry::SetLoadedAmmo(int32 NewLoadedAmmo)
{
    if (LoadedAmmo != NewLoadedAmmo)
    {
        LoadedAmmo = NewLoadedAmmo;

        if (ParentInventory.IsValid())
        {
            ParentInventory.Get()->SetItemRequiresUpdate(*this);
        }

        bIsDirty = true;
    }
}

void FFortItemEntry::SetStateValue(EFortItemEntryState::Type StateType, int32 IntValue)
{
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

    if (ParentInventory.IsValid())
    {
        ParentInventory.Get()->SetItemRequiresUpdate(*this);
    }

    bIsDirty = true;
}
