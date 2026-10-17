#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Items/FortResourceItemDefinition.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto InitializeMcpProfile = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Initialized MCP profile for %s")))->BeginAddress, &AFortPlayerController::InitializeMcpProfile);

static auto ActivateSlot = DetourCall([]
{
    auto Reference = Scanner::FindStringReference(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase"));
    auto End = Scanner::FindFunctionEnd(Scanner::FindFunction(Reference));
    auto bUpdatePreviousFocusedSlot = false;

    for (auto Address = Reference; Address < End;)
    {
        hde64s Instruction;
        auto Length = hde64_disasm(Address, &Instruction);

        if (Instruction.opcode == 0xE8 && bUpdatePreviousFocusedSlot)
            return Address;

        bUpdatePreviousFocusedSlot |= !memcmp(Address, "\xC6\x44\x24\x20\x01", 5);
        Address += Length;
    }

    return (const uint8_t*)nullptr;
}(), &AFortPlayerController::ActivateSlot);

bool AFortPlayerController::InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized)
{
    static auto ServerExecuteInventoryItem = []
    {
        auto Function = Scanner::FindFunction(std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerExecuteInventoryItem")))->GetNativeFunc()));
        auto End = Scanner::FindFunctionEnd(Function);
        int32_t ServerExecuteInventoryItem_Implementation = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                ServerExecuteInventoryItem_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

        return DetourVTable((void**)(VTable + ServerExecuteInventoryItem_Implementation), &AFortPlayerController::ServerExecuteInventoryItem_Implementation);
    }();

    static auto RemoveInventoryItem = []
    {
        auto Function = Scanner::FindFunction(std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")))->GetNativeFunc()));
        auto End = Scanner::FindFunctionEnd(Function);
        int32_t ServerRemoveInventoryItem_Implementation = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                ServerRemoveInventoryItem_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        int32_t InventoryOwner = 0;
        int32_t RemoveInventoryItem = 0;

        for (auto Address = *(const uint8_t**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation); !RemoveInventoryItem;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x81 && Instruction.modrm_mod == 3 && Instruction.modrm_reg == 0)
                InventoryOwner = (int32_t)Instruction.imm.imm32;

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                RemoveInventoryItem = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        static auto InventoryOwnerOffset = InventoryOwner;

        auto InventoryOwnerRemoveInventoryItem = +[](uint8_t* InventoryOwnerInterface, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
        {
            return ((AFortPlayerController*)(InventoryOwnerInterface - InventoryOwnerOffset))->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars);
        };

        return DetourVTable((void**)(*(uint8_t**)(PlayerController + InventoryOwner) + RemoveInventoryItem), (void*)InventoryOwnerRemoveInventoryItem);
    }();

    static auto SetLoadedAmmo = []
    {
        auto Function = Scanner::FindFunction(std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")))->GetNativeFunc()));
        auto End = Scanner::FindFunctionEnd(Function);
        int32_t ServerRemoveInventoryItem_Implementation = 0;

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xFF && (Instruction.modrm_reg == 2 || Instruction.modrm_reg == 4) && Instruction.modrm_mod == 2)
                ServerRemoveInventoryItem_Implementation = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        int32_t InventoryOwner = 0;
        int32_t RemoveInventoryItem = 0;

        for (auto Address = *(const uint8_t**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation); !RemoveInventoryItem;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0x81 && Instruction.modrm_mod == 3 && Instruction.modrm_reg == 0)
                InventoryOwner = (int32_t)Instruction.imm.imm32;

            if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 2 && Instruction.modrm_mod == 2)
                RemoveInventoryItem = (int32_t)Instruction.disp.disp32;

            Address += Length;
        }

        auto InventoryOwnerVTable = *(const uint8_t***)(PlayerController + InventoryOwner);

        for (auto Index = 0;; Index++)
        {
            auto Function = Scanner::FindFunction(InventoryOwnerVTable[Index]);

            if (!Function)
                continue;

            auto End = Scanner::FindFunctionEnd(Function);

            for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
            {
                hde64s Instruction;
                auto Length = hde64_disasm(Address, &Instruction);

                if (Instruction.opcode == 0xFF && Instruction.modrm_reg == 4 && Instruction.modrm_mod == 2 && Instruction.rex_b)
                {
                    auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWorldItem"));

                    return DetourVTable((void**)(VTable + (int32_t)Instruction.disp.disp32), &UFortWorldItem::SetLoadedAmmo);
                }

                Address += Length;
            }
        }
    }();

    auto bInitialized = (this->*::InitializeMcpProfile)(OnMcpProfilesInitialized);

    if (HasAuthority() && !GetQuickBars())
    {
        FActorSpawnParameters SpawnInfo;
        SpawnInfo.Owner = this;

        GetQuickBars() = (AFortQuickBars*)GetWorld()->SpawnActor(AFortQuickBars::StaticClass(), nullptr, SpawnInfo);
    }

    return bInitialized;
}

void AFortPlayerController::AddItemToQuickBars(UFortItemDefinition* ItemDefinition, EFortQuickBars QuickBarType, int32 SlotIndex)
{
    static auto AddItemToQuickBars = []
    {
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:AddItemToQuickBars")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                return (void (*)(AFortPlayerController*, UFortItemDefinition*, EFortQuickBars, int32))(Address + Length + (int32_t)Instruction.imm.imm32);

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }
    }();

    AddItemToQuickBars(this, ItemDefinition, QuickBarType, SlotIndex);
}

void AFortPlayerController::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    if (GetQuickBars())
        GetQuickBars()->ActivateSlot(InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);
}

UFortItem* AFortPlayerController::K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const
{
    static auto K2_FindExistingItemForDefinition = []
    {
        auto bFinished = false;

        for (auto Address = std::bit_cast<const uint8_t*>(((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:K2_FindExistingItemForDefinition")))->GetNativeFunc());;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (bFinished && Instruction.opcode == 0xE8)
                return (UFortItem* (*)(const AFortPlayerController*, const UFortItemDefinition*, bool))(Address + Length + (int32_t)Instruction.imm.imm32);

            bFinished |= Instruction.opcode == 0x89 && Instruction.rex_w && Instruction.modrm_mod == 1 && Instruction.modrm_rm != 4 && Instruction.disp.disp8 == 0x20;
            Address += Length;
        }
    }();

    return K2_FindExistingItemForDefinition(this, ItemDefinition, bInStorageVault);
}

void AFortPlayerController::ServerExecuteInventoryItem_Implementation(FGuid ItemGuid)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    for (auto Item : WorldInventory->GetInventory().ItemInstances)
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            if (auto ItemDefinition = Cast<UFortWorldItemDefinition>(Item->GetItemDefinitionBP()))
                ItemDefinition->ServerExecute(Item, this);

            return;
        }
    }
}

UClass* AFortPlayerController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController"));

    return Class;
}

void AFortPlayerController::ClientReportDamagedResourceBuilding(ABuildingSMActor* BuildingSMActor, EFortResourceType PotentialResourceType, int32 PotentialResourceCount, bool bDestroyed, bool bJustHitWeakspot)
{
    static auto ClientReportDamagedResourceBuilding = FindFunctionChecked(FName(TEXT("ClientReportDamagedResourceBuilding")));

    struct
    {
        ABuildingSMActor* BuildingSMActor;
        EFortResourceType PotentialResourceType;
        int32 PotentialResourceCount;
        bool bDestroyed;
        bool bJustHitWeakspot;
    } Parms{ BuildingSMActor, PotentialResourceType, PotentialResourceCount, bDestroyed, bJustHitWeakspot };

    ProcessEvent(ClientReportDamagedResourceBuilding, &Parms);
}

bool AFortPlayerController::RemoveInventoryItem(const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return false;

    auto& Inventory = WorldInventory->GetInventory();

    for (int32 Index = 0; Index < Inventory.ReplicatedEntries.Num(); Index++)
    {
        auto& ItemEntry = Inventory.ReplicatedEntries[Index];

        if (ItemEntry.ItemGuid != ItemGuid)
            continue;

        auto ItemDefinition = ItemEntry.ItemDefinition;

        if (ItemEntry.Count > Count)
        {
            ItemEntry.SetCount(ItemEntry.Count - Count);
            Inventory.MarkItemDirty(ItemEntry);

            if (bForceRemoveFromQuickBars)
                RemoveItemFromQuickBars(ItemDefinition);
        }
        else
        {
            RemoveItemFromQuickBars(ItemDefinition);

            Inventory.ReplicatedEntries.RemoveAt(Index);
            Inventory.MarkArrayDirty();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        return true;
    }

    return false;
}

void AFortPlayerController::RemoveItemFromQuickBars(UFortItemDefinition* ItemDefinition)
{
    static auto RemoveItemFromQuickBars = FindFunctionChecked(FName(TEXT("RemoveItemFromQuickBars")));

    struct
    {
        UFortItemDefinition* ItemDefinition;
    } Parms{ ItemDefinition };

    ProcessEvent(RemoveItemFromQuickBars, &Parms);
}

AFortQuickBars*& AFortPlayerController::GetQuickBars()
{
    static auto QuickBars = GetClass()->FindPropertyByName(FName(TEXT("QuickBars")));

    return *QuickBars->ContainerPtrToValuePtr<AFortQuickBars*>(this);
}

AFortInventory*& AFortPlayerController::GetWorldInventory()
{
    static auto WorldInventory = GetClass()->FindPropertyByName(FName(TEXT("WorldInventory")));

    return *WorldInventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}

UFortRegisteredPlayerInfo*& AFortPlayerController::GetMyPlayerInfo()
{
    static auto MyPlayerInfo = GetClass()->FindPropertyByName(FName(TEXT("MyPlayerInfo")));

    return *MyPlayerInfo->ContainerPtrToValuePtr<UFortRegisteredPlayerInfo*>(this);
}
