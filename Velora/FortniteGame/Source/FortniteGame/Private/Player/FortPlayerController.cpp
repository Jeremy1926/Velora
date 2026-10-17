#include "pch.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"
#include "hde/hde64.h"

static auto InitializeMcpProfile = Detour(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("Initialized MCP profile for %s")))->BeginAddress, &AFortPlayerController::InitializeMcpProfile);

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
