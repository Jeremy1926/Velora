#include "pch.h"
#include "FortControllerComponent_CampsiteAccountItem.h"
#include "Player/FortPlayerController.h"
#include "Pawns/FortPlayerPawn.h"
#include "Weapons/FortWeapon.h"
#include "Items/FortWorldItem.h"
#include "Items/FortPickup.h"
#include "Items/InventoryManagementLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptInterface.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortControllerComponent_CampsiteAccountItem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem"));

    return Class;
}

void UFortControllerComponent_CampsiteAccountItem::execStashCurrentlyHeldItemAndRemoveFromInventory(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execStashCurrentlyHeldItemAndRemoveFromInventory, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:StashCurrentlyHeldItemAndRemoveFromInventory"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:StashCurrentlyHeldItemAndRemoveFromInventory")));

    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = StashCurrentlyHeldItemAndRemoveFromInventory(StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execHasCurrentlyStashedItem(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execHasCurrentlyStashedItem, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:HasCurrentlyStashedItem"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:HasCurrentlyStashedItem")));

    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = HasCurrentlyStashedItem(StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execClearStashedItemAndGiveToPlayer(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execClearStashedItemAndGiveToPlayer, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:ClearStashedItemAndGiveToPlayer"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:ClearStashedItemAndGiveToPlayer")));

    AActor* SourceActor = nullptr;
    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&SourceActor);
    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = ClearStashedItemAndGiveToPlayer(SourceActor, StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execSwapStashedItem(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execSwapStashedItem, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:SwapStashedItem"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:SwapStashedItem")));

    AActor* SourceActor = nullptr;
    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&SourceActor);
    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = SwapStashedItem(SourceActor, StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execClearStashedItem(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execClearStashedItem, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:ClearStashedItem"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:ClearStashedItem")));

    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    ClearStashedItem(StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execWriteStashedItem(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execWriteStashedItem, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItem"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItem")));

    UFortWorldItem* Item = nullptr;
    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&Item);
    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    WriteStashedItem(Item, StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::execWriteStashedItemFromEntry(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execWriteStashedItemFromEntry, Anchor::Hook()
        .If(StaticClass())
        .Exec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItemFromEntry"))
        .Find("add [*+0x20], *; call 0x*; mov rcx, [rsp+0x*]; xor rcx, rsp; call 0x*")
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItemFromEntry")));

    FFortItemEntry TemporaryItemEntry(nullptr, 0, 0);
    auto& ItemEntry = Stack.StepCompiledInRef<FFortItemEntry>(&TemporaryItemEntry);
    int32 StashedItemIndex = 0;

    Stack.StepCompiledIn(&StashedItemIndex);
    Stack.Code += !!Stack.Code;

    WriteStashedItemFromEntry(ItemEntry, StashedItemIndex);
}

bool UFortControllerComponent_CampsiteAccountItem::StashCurrentlyHeldItemAndRemoveFromInventory(int32 StashedItemIndex)
{
    auto PlayerController = GetPlayerController();
    auto Item = GetCurrentlyHeldItem();

    if (!PlayerController || !Item || StashedItemIndex < 0)
        return false;

    const FGuid ItemGuid = Item->GetItemGuid();
    const int32 Count = Item->GetItemEntry().GetCount();

    WriteStashedItem(Item, StashedItemIndex);

    return PlayerController->RemoveInventoryItem(ItemGuid, Count, true);
}

bool UFortControllerComponent_CampsiteAccountItem::HasCurrentlyStashedItem(int32 StashedItemIndex)
{
    auto& CurrentlyStashedItemEntries = GetCurrentlyStashedItemEntries();

    return StashedItemIndex >= 0 && StashedItemIndex < CurrentlyStashedItemEntries.Num() && CurrentlyStashedItemEntries[StashedItemIndex].GetItemDefinition() && CurrentlyStashedItemEntries[StashedItemIndex].GetCount() > 0;
}

bool UFortControllerComponent_CampsiteAccountItem::ClearStashedItemAndGiveToPlayer(const AActor* SourceActor, int32 StashedItemIndex)
{
    if (!GetPlayerController() || !HasCurrentlyStashedItem(StashedItemIndex))
        return false;

    FFortItemEntry StashedItemEntry(GetCurrentlyStashedItemEntries()[StashedItemIndex]);

    ClearStashedItem(StashedItemIndex);
    GiveStashedItemToPlayer(SourceActor, StashedItemEntry);

    return true;
}

bool UFortControllerComponent_CampsiteAccountItem::SwapStashedItem(const AActor* SourceActor, int32 StashedItemIndex)
{
    auto PlayerController = GetPlayerController();
    auto Item = GetCurrentlyHeldItem();

    if (!PlayerController || !Item || !HasCurrentlyStashedItem(StashedItemIndex))
        return false;

    FFortItemEntry StashedItemEntry(GetCurrentlyStashedItemEntries()[StashedItemIndex]);
    const FGuid ItemGuid = Item->GetItemGuid();
    const int32 Count = Item->GetItemEntry().GetCount();

    WriteStashedItem(Item, StashedItemIndex);
    PlayerController->RemoveInventoryItem(ItemGuid, Count, true);
    GiveStashedItemToPlayer(SourceActor, StashedItemEntry);

    return true;
}

void UFortControllerComponent_CampsiteAccountItem::ClearStashedItem(int32 StashedItemIndex)
{
    auto& CurrentlyStashedItemEntries = GetCurrentlyStashedItemEntries();

    if (StashedItemIndex < 0 || StashedItemIndex >= CurrentlyStashedItemEntries.Num())
        return;

    CurrentlyStashedItemEntries[StashedItemIndex].~FFortItemEntry();
    new (&CurrentlyStashedItemEntries[StashedItemIndex]) FFortItemEntry(nullptr, 0, 0);
}

void UFortControllerComponent_CampsiteAccountItem::WriteStashedItem(UFortWorldItem* Item, int32 StashedItemIndex)
{
    if (Item)
        WriteStashedItemFromEntry(Item->GetItemEntry(), StashedItemIndex);
}

void UFortControllerComponent_CampsiteAccountItem::WriteStashedItemFromEntry(const FFortItemEntry& ItemEntry, int32 StashedItemIndex)
{
    auto& CurrentlyStashedItemEntries = GetCurrentlyStashedItemEntries();

    if (StashedItemIndex < 0)
        return;

    while (CurrentlyStashedItemEntries.Num() <= StashedItemIndex)
        new (CurrentlyStashedItemEntries) FFortItemEntry(nullptr, 0, 0);

    CurrentlyStashedItemEntries[StashedItemIndex].~FFortItemEntry();
    new (&CurrentlyStashedItemEntries[StashedItemIndex]) FFortItemEntry(ItemEntry);
}

UFortWorldItem* UFortControllerComponent_CampsiteAccountItem::GetCurrentlyHeldItem()
{
    auto PlayerController = GetPlayerController();
    auto Pawn = PlayerController ? PlayerController->GetMyFortPawn() : nullptr;
    auto Weapon = Pawn ? Pawn->GetCurrentWeapon() : nullptr;
    auto WorldInventory = Weapon ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return nullptr;

    for (auto Item : WorldInventory->GetInventory().GetItemInstances())
    {
        if (Item && Item->GetItemGuid() == Weapon->GetItemEntryGuid())
            return Item;
    }

    return nullptr;
}

void UFortControllerComponent_CampsiteAccountItem::GiveStashedItemToPlayer(const AActor* SourceActor, const FFortItemEntry& ItemEntry)
{
    static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

    auto PlayerController = GetPlayerController();
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return;

    if (WorldInventory->GetOverflowFromAddingItem(ItemEntry) <= 0)
    {
        TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
        InventoryOwner.SetObject(PlayerController);
        InventoryOwner.SetInterface(PlayerController->GetInterfaceAddress(FortInventoryOwnerInterface));

        UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(InventoryOwner, ItemEntry);
        return;
    }

    auto SpawnSource = SourceActor ? SourceActor : (const AActor*)PlayerController->GetMyFortPawn();

    if (SpawnSource)
        AFortPickup::CreateFromData(GetWorld(), ItemEntry, SpawnSource->GetActorLocation(), SpawnSource->GetActorRotation(), PlayerController);
}

AFortPlayerController* UFortControllerComponent_CampsiteAccountItem::GetPlayerController() const
{
    auto PlayerController = Cast<AFortPlayerController>(GetOwner());

    return PlayerController && PlayerController->HasAuthority() ? PlayerController : nullptr;
}

TArray<FFortItemEntry>& UFortControllerComponent_CampsiteAccountItem::GetCurrentlyStashedItemEntries()
{
    static auto CurrentlyStashedItemEntries = StaticClass()->FindPropertyByName(FName(TEXT("CurrentlyStashedItemEntries")));

    return *CurrentlyStashedItemEntries->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}
