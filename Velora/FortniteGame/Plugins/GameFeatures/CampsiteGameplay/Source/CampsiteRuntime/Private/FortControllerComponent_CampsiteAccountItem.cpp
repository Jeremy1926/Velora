#include "pch.h"
#include "FortControllerComponent_CampsiteAccountItem.h"
#include "Player/FortPlayerController.h"
#include "Pawns/FortPlayerPawn.h"
#include "Weapons/FortWeapon.h"
#include "Items/FortWorldItem.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortCampsiteAccountItem.h"
#include "Online/FortMcpProfile.h"
#include "Items/FortPickup.h"
#include "Items/InventoryManagementLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
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

void UFortControllerComponent_CampsiteAccountItem::BeginPlay()
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, BeginPlay, Anchor::Hook()
        .If(StaticClass())
        .VirtualMatch(TEXT("/Script/Engine.Default__ActorComponent"), "mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rax, [rcx+0x10]; mov rdi, rcx; mov ecx, [rax+0x*]; bt ecx, 0x12")
        .LogSlot(TEXT("UActorComponent::BeginPlay"))
        .InVTable(TEXT("/Script/CampsiteRuntime.Default__FortControllerComponent_CampsiteAccountItem")));

    Original();

    BindOnAthenaProfileInitialized();
}

void UFortControllerComponent_CampsiteAccountItem::execOnAthenaProfileInitialized(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execOnAthenaProfileInitialized, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:OnAthenaProfileInitialized"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:OnAthenaProfileInitialized")));

    Stack.Code += !!Stack.Code;

    OnAthenaProfileInitialized();
}

void UFortControllerComponent_CampsiteAccountItem::execCacheAccountItemData(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_CampsiteAccountItem, execCacheAccountItemData, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:CacheAccountItemData"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:CacheAccountItemData")));

    Stack.Code += !!Stack.Code;

    CacheAccountItemData();
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

    static auto bHasPlayerController = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItem")))->FindPropertyByName(FName(TEXT("PlayerController"))) != nullptr;

    AFortPlayerController* PlayerController = nullptr;
    UFortWorldItem* Item = nullptr;
    int32 StashedItemIndex = 0;

    if (bHasPlayerController)
        Stack.StepCompiledIn(&PlayerController);

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

    static auto bHasPlayerController = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.FortControllerComponent_CampsiteAccountItem:WriteStashedItemFromEntry")))->FindPropertyByName(FName(TEXT("PlayerController"))) != nullptr;

    AFortPlayerController* PlayerController = nullptr;

    if (bHasPlayerController)
        Stack.StepCompiledIn(&PlayerController);

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

void UFortControllerComponent_CampsiteAccountItem::BindOnAthenaProfileInitialized()
{
    auto PlayerController = GetPlayerController();

    if (!PlayerController)
        return;

    if (PlayerController->GetAthenaProfile())
    {
        OnAthenaProfileInitialized();
        return;
    }

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("OnAthenaProfileInitialized")));

    PlayerController->GetOnMcpProfilesInitializedEvent().AddUnique(Delegate);
}

void UFortControllerComponent_CampsiteAccountItem::OnAthenaProfileInitialized()
{
    CacheAccountItemData();
}

void UFortControllerComponent_CampsiteAccountItem::CacheAccountItemData()
{
    static auto CampsiteAccountItemDefinition = StaticClass()->FindPropertyByName(FName(TEXT("CampsiteAccountItemDefinition")));

    auto PlayerController = GetPlayerController();
    auto AthenaProfile = PlayerController ? PlayerController->GetAthenaProfile() : nullptr;
    auto AccountItemDefinition = *CampsiteAccountItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);

    if (!AthenaProfile || !AccountItemDefinition)
        return;

    TArray<UFortAccountItem*> AccountItems;
    AthenaProfile->GetItemInstancesForTemplate(AccountItemDefinition->GetPersistentName(), AccountItems);

    auto AccountItem = AccountItems.Num() ? Cast<UFortCampsiteAccountItem>(AccountItems[0]) : nullptr;

    if (!AccountItem)
        return;

    auto& CampsiteAccountData = GetCampsiteAccountData();
    auto& CurrentlyStashedItemEntries = GetCurrentlyStashedItemEntries();

    FCampsiteAccountData::StaticStruct()->CopyScriptStruct(&CampsiteAccountData, &AccountItem->GetCampsiteAccountData());

    while (CurrentlyStashedItemEntries.Num() < CampsiteAccountData.GetStashedItems().Num())
        new (CurrentlyStashedItemEntries) FFortItemEntry(nullptr, 0, 0);

    CampsiteAccountData.CopyFromMCPData(CurrentlyStashedItemEntries);
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

FCampsiteAccountData& UFortControllerComponent_CampsiteAccountItem::GetCampsiteAccountData()
{
    static auto CampsiteAccountData = StaticClass()->FindPropertyByName(FName(TEXT("CampsiteAccountData")));

    return *CampsiteAccountData->ContainerPtrToValuePtr<FCampsiteAccountData>(this);
}
