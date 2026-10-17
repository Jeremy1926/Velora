#include "pch.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "hde/hde64.h"

FFortItemList& AFortInventory::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<FFortItemList>(this);
}

void AFortInventory::HandleInventoryLocalUpdate()
{
    static auto HandleInventoryLocalUpdate = []
    {
        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventory:HandleInventoryLocalUpdate")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 || Instruction.opcode == 0xE9)
                return (void (*)(AFortInventory*))(Address + Length + (int32_t)Instruction.imm.imm32);

            Address += Length;
        }
    }();

    HandleInventoryLocalUpdate(this);
}

void AFortInventory::SetItemRequiresUpdate(FFortItemEntry& ItemEntry)
{
    static auto SetItemRequiresUpdate = []
    {
        const uint8_t* StorageVaultStoreItem = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortOutpostFunctionLibrary:StorageVaultStoreItem")))->GetNativeFunc()); !StorageVaultStoreItem;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                StorageVaultStoreItem = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        const uint8_t* SetCount = nullptr;
        auto Calls = 0;

        for (auto Address = StorageVaultStoreItem; !SetCount;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8 && ++Calls == 3)
                SetCount = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        const uint8_t* SetItemRequiresUpdate = nullptr;
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(SetCount));

        for (auto Address = SetCount; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                SetItemRequiresUpdate = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (void (*)(AFortInventory*, FFortItemEntry&))SetItemRequiresUpdate;
    }();

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
