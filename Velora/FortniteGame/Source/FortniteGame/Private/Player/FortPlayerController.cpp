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

static auto InitializeMcpProfile = Detour(TEXT("AFortPlayerController::InitializeMcpProfile"), Scanner::String(TEXT("Initialized MCP profile for %s")).Start().Get(), &AFortPlayerController::InitializeMcpProfile);

static auto ActivateSlot = DetourCall(TEXT("AFortPlayerController::ActivateSlot"), Scanner::String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase")).Find("mov byte ptr [rsp+0x20], 0x1").Find("call 0x*").Get(), &AFortPlayerController::ActivateSlot);

static auto RemoveAllInventoryItems = DetourCall(TEXT("AFortPlayerController::RemoveAllInventoryItems"), Scanner::String(TEXT("AFortGameModeAthena::StartAircraftPhase: Initiating aircraft phase")).Find("mov byte ptr [rsp+0x20], 0x1").Find("call 0x*", 3).Get(), &AFortPlayerController::RemoveAllInventoryItems);

bool AFortPlayerController::InitializeMcpProfile(const FOnMcpProfilesInitialized& OnMcpProfilesInitialized)
{
    static auto ServerExecuteInventoryItem = []
    {
        auto ServerExecuteInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerExecuteInventoryItem")).Find("call [rax+*]", 2).Disp();
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

        return DetourVTable(TEXT("AFortPlayerController::ServerExecuteInventoryItem_Implementation"), (void**)(VTable + ServerExecuteInventoryItem_Implementation), &AFortPlayerController::ServerExecuteInventoryItem_Implementation);
    }();

    static auto RemoveInventoryItem = []
    {
        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        auto ServerRemoveInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")).Find("call [rax+*]", 2).Disp();
        auto RemoveInventoryItem = Scanner::At(*(void**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation)).Find("call [rax+*]");
        static auto InventoryOwnerOffset = RemoveInventoryItem.Last("add rcx, 0x*").Imm();

        auto InventoryOwnerRemoveInventoryItem = +[](uint8_t* InventoryOwnerInterface, const FGuid& ItemGuid, int32 Count, bool bForceRemoveFromQuickBars)
        {
            return ((AFortPlayerController*)(InventoryOwnerInterface - InventoryOwnerOffset))->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars);
        };

        return DetourVTable(TEXT("AFortPlayerController::RemoveInventoryItem"), (void**)(*(uint8_t**)(PlayerController + InventoryOwnerOffset) + RemoveInventoryItem.Disp()), (void*)InventoryOwnerRemoveInventoryItem);
    }();

    static auto SetLoadedAmmo = []
    {
        auto PlayerController = (uint8_t*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));
        auto ServerRemoveInventoryItem_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerController:ServerRemoveInventoryItem")).Find("call [rax+*]", 2).Disp();
        auto InventoryOwner = Scanner::At(*(void**)(*(uint8_t**)PlayerController + ServerRemoveInventoryItem_Implementation)).Find("call [rax+*]").Last("add rcx, 0x*").Imm();
        auto InventoryOwnerVTable = *(const uint8_t***)(PlayerController + InventoryOwner);
        auto Index = 0;

        while (!Scanner::At(InventoryOwnerVTable[Index]).Start().Has("jmp [r8+*]"))
            Index++;

        auto SetLoadedAmmo = Scanner::At(InventoryOwnerVTable[Index]).Start().Find("jmp [r8+*]").Disp();
        auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWorldItem"));

        return DetourVTable(TEXT("UFortWorldItem::SetLoadedAmmo"), (void**)(VTable + SetLoadedAmmo), &UFortWorldItem::SetLoadedAmmo);
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
    static auto AddItemToQuickBars = (void (*)(AFortPlayerController*, UFortItemDefinition*, EFortQuickBars, int32))Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerController:AddItemToQuickBars")).Get();

    AddItemToQuickBars(this, ItemDefinition, QuickBarType, SlotIndex);
}

void AFortPlayerController::ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot)
{
    if (GetQuickBars())
        GetQuickBars()->ActivateSlot(InQuickBar, Slot, ActivateDelay, bUpdatePreviousFocusedSlot);
}

UFortItem* AFortPlayerController::K2_FindExistingItemForDefinition(const UFortItemDefinition* ItemDefinition, bool bInStorageVault) const
{
    static auto K2_FindExistingItemForDefinition = (UFortItem* (*)(const AFortPlayerController*, const UFortItemDefinition*, bool))Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerController:K2_FindExistingItemForDefinition")).Get();

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

void AFortPlayerController::RemoveAllInventoryItems()
{
    auto WorldInventory = GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& ReplicatedEntries = WorldInventory->GetInventory().ReplicatedEntries;

    for (int32 Index = ReplicatedEntries.Num() - 1; Index >= 0; Index--)
    {
        const FGuid ItemGuid = ReplicatedEntries[Index].ItemGuid;

        RemoveInventoryItem(ItemGuid, ReplicatedEntries[Index].Count, true);
    }
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
