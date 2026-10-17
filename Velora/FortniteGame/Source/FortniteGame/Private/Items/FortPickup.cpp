#include "pch.h"
#include "Items/FortPickup.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortQuickBars.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Pawns/FortPawn.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "hde/hde64.h"

UClass* AFortPickup::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPickup"));

    return Class;
}

AFortPickup* AFortPickup::CreateFromData(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& SpawnLocation, const FRotator& SpawnRotation, AFortPlayerController* OptionalOwnerPC, UClass* OverrideClass)
{
    static auto CreateFromData = []
    {
        const uint8_t* SpawnMissionItemPickup = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")))->GetNativeFunc()); !SpawnMissionItemPickup;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                SpawnMissionItemPickup = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        auto bOverrideClass = false;

        for (auto Address = SpawnMissionItemPickup;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bOverrideClass && Instruction.opcode == 0xE8)
                return (AFortPickup* (*)(UWorld*, const FFortItemEntry&, const FVector&, const FRotator&, AFortPlayerController*, UClass*))(Address + Length + (int32_t)Instruction.imm.imm32);

            bOverrideClass |= Instruction.opcode == 0xC7 && Instruction.modrm_mod == 1 && Instruction.modrm_rm == 4 && Instruction.disp.disp8 == 0x28;
            Address += Length;
        }
    }();

    return CreateFromData(World, ItemEntry, SpawnLocation, SpawnRotation, OptionalOwnerPC, OverrideClass);
}

void AFortPickup::TossPickup(const FVector& FinalLocation, AFortPawn* ItemOwner, int32 OverrideMaxStackCount, bool bToss)
{
    static auto TossPickup = FindFunctionChecked(FName(TEXT("TossPickup")));

    struct
    {
        FVector FinalLocation;
        AFortPawn* ItemOwner;
        int32 OverrideMaxStackCount;
        bool bToss;
    } Parms{ FinalLocation, ItemOwner, OverrideMaxStackCount, bToss };

    ProcessEvent(TossPickup, &Parms);
}

void AFortPickup::GivePickupTo(IFortInventoryOwnerInterface* InventoryOwner)
{
    if (HasAuthority() && InventoryOwner)
    {
        GivePickupItemsToInventoryOwner(InventoryOwner);
        Destroy();
    }
}

void AFortPickup::GivePickupItemsToInventoryOwner(IFortInventoryOwnerInterface* InventoryOwner)
{
    auto PickupTarget = GetPickupLocationData().PickupTarget;
    auto PlayerController = PickupTarget ? Cast<AFortPlayerControllerAthena>(PickupTarget->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return;

    auto GiveItemEntry = [&](const FFortItemEntry& PickupItemEntry)
    {
        auto ItemDefinition = PickupItemEntry.ItemDefinition;

        if (!ItemDefinition)
            return;

        auto& Inventory = WorldInventory->GetInventory();
        const int32 MaxStackSize = ItemDefinition->GetMaxStackSize();
        int32 NumberToGive = PickupItemEntry.Count;
        auto bCanAddStack = true;

        for (auto& ItemEntry : Inventory.ReplicatedEntries)
        {
            if (ItemEntry.ItemDefinition != ItemDefinition)
                continue;

            const int32 NumberToStack = FMath::Min(NumberToGive, MaxStackSize - ItemEntry.Count);

            if (NumberToStack > 0)
            {
                ItemEntry.SetCount(ItemEntry.Count + NumberToStack);
                Inventory.MarkItemDirty(ItemEntry);

                NumberToGive -= NumberToStack;
            }

            bCanAddStack = ItemDefinition->DoesAllowMultipleStacks();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        while (bCanAddStack && NumberToGive > 0)
        {
            auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, FMath::Min(NumberToGive, MaxStackSize), PickupItemEntry.Level);
            ItemEntry->LoadedAmmo = PickupItemEntry.LoadedAmmo;
            ItemEntry->Durability = PickupItemEntry.Durability;

            NumberToGive -= ItemEntry->Count;

            Inventory.MarkItemDirty(*ItemEntry);
            WorldInventory->HandleInventoryLocalUpdate();

            PlayerController->AddItemToQuickBars(ItemDefinition, EFortQuickBars::Max_None, -1);
        }

        if (NumberToGive > 0)
        {
            FFortItemEntry LeftoverItemEntry(ItemDefinition, NumberToGive, PickupItemEntry.Level);
            LeftoverItemEntry.LoadedAmmo = PickupItemEntry.LoadedAmmo;
            LeftoverItemEntry.Durability = PickupItemEntry.Durability;

            const FVector Location = PickupTarget->GetActorLocation();

            if (auto Pickup = CreateFromData(GetWorld(), LeftoverItemEntry, Location, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, GetClass()))
                Pickup->TossPickup(Location, PickupTarget, 0, true);
        }
    };

    auto& MultiItemPickupEntries = GetMultiItemPickupEntries();

    if (MultiItemPickupEntries.Num())
    {
        for (auto& PickupItemEntry : MultiItemPickupEntries)
            GiveItemEntry(PickupItemEntry);
    }
    else
    {
        GiveItemEntry(GetPrimaryPickupItemEntry());
    }
}

FFortItemEntry& AFortPickup::GetPrimaryPickupItemEntry()
{
    static auto PrimaryPickupItemEntry = GetClass()->FindPropertyByName(FName(TEXT("PrimaryPickupItemEntry")));

    return *PrimaryPickupItemEntry->ContainerPtrToValuePtr<FFortItemEntry>(this);
}

TArray<FFortItemEntry>& AFortPickup::GetMultiItemPickupEntries()
{
    static auto MultiItemPickupEntries = GetClass()->FindPropertyByName(FName(TEXT("MultiItemPickupEntries")));

    return *MultiItemPickupEntries->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

FFortPickupLocationData& AFortPickup::GetPickupLocationData()
{
    static auto PickupLocationData = GetClass()->FindPropertyByName(FName(TEXT("PickupLocationData")));

    return *PickupLocationData->ContainerPtrToValuePtr<FFortPickupLocationData>(this);
}

bool& AFortPickup::GetTossedFromContainer()
{
    static auto bTossedFromContainer = GetClass()->FindPropertyByName(FName(TEXT("bTossedFromContainer")));

    return *bTossedFromContainer->ContainerPtrToValuePtr<bool>(this);
}

void AFortPickup::SetPickupTarget(AFortPawn* InPickupTarget, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    static auto SetPickupTarget = []
    {
        const uint8_t* SpawnMissionItemPickup = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")))->GetNativeFunc()); !SpawnMissionItemPickup;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                SpawnMissionItemPickup = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        const uint8_t* SetPickupTarget = nullptr;
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(SpawnMissionItemPickup));

        for (auto Address = SpawnMissionItemPickup; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                SetPickupTarget = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        return (void (*)(AFortPickup*, AFortPawn*, float, const FVector&, bool))SetPickupTarget;
    }();

    SetPickupTarget(this, InPickupTarget, InFlyTime, InStartDirection, bPlayPickupSound);
}

float AFortPickup::GetFlyTime() const
{
    static auto GetFlyTime = []
    {
        const uint8_t* SpawnMissionItemPickup = nullptr;
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")))->GetNativeFunc()); !SpawnMissionItemPickup;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                SpawnMissionItemPickup = Address + Length + (int32_t)Instruction.imm.imm32;

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }

        const uint8_t* SetPickupTarget = nullptr;
        auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(SpawnMissionItemPickup));

        for (auto Address = SpawnMissionItemPickup; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
                SetPickupTarget = Address + Length + (int32_t)Instruction.imm.imm32;

            Address += Length;
        }

        auto bCompared = false;

        for (auto Address = SetPickupTarget;;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bCompared && Instruction.opcode == 0xE8)
                return (float (*)(const AFortPickup*))(Address + Length + (int32_t)Instruction.imm.imm32);

            bCompared |= Instruction.opcode == 0x0F && Instruction.opcode2 == 0x2F;
            Address += Length;
        }
    }();

    return GetFlyTime(this);
}
