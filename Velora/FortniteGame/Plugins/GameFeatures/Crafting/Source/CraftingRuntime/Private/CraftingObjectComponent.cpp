#include "pch.h"
#include "CraftingObjectComponent.h"
#include "FortKismetLibrary.h"
#include "GameFramework/Actor.h"
#include "Items/FortPickup.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Items/InventoryManagementLibrary.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/ScriptInterface.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UCraftingObjectComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.CraftingObjectComponent"));

    return Class;
}

void UCraftingObjectComponent::StartCrafting(AFortPlayerController* Instigator, const FName& FormulaRow)
{
    static auto StartCrafting = Anchor::Function<void (*)(UCraftingObjectComponent*, AFortPlayerController*, const FName*)>(TEXT("UCraftingObjectComponent::StartCrafting"))
        .Smallest(TEXT("Event.Athena.Crafting.CraftingFailed"))
        .Start()
        .Get();

    StartCrafting(this, Instigator, &FormulaRow);
}

void UCraftingObjectComponent::PickupItemAndStartCrafting(AFortPlayerController* Instigator, AFortPickup* Pickup, const FName& FormulaRow)
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || !Pickup || GetCraftingObjectState() != ECraftingObjectState::Idle)
        return;

    static auto PendingPickupCraftingItem = StaticClass()->FindPropertyByName(FName(TEXT("PendingPickupCraftingItem")));
    static auto PendingPickupCraftingFormula = StaticClass()->FindPropertyByName(FName(TEXT("PendingPickupCraftingFormula")));
    static auto OnPickup = Pickup->GetClass()->FindPropertyByName(FName(TEXT("OnPickup")));

    if (!PendingPickupCraftingItem || !PendingPickupCraftingFormula || !OnPickup)
        return;

    *PendingPickupCraftingItem->ContainerPtrToValuePtr<AFortPickup*>(this) = Pickup;
    PendingPickupCraftingFormula->SetPropertyValue_InContainer<FName>(this, FormulaRow);

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("HandlePickupCraftingItemPickedUp")));

    OnPickup->ContainerPtrToValuePtr<FMulticastScriptDelegate>(Pickup)->AddUnique(Delegate);
}

void UCraftingObjectComponent::CancelCrafting()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || GetCraftingObjectState() != ECraftingObjectState::Crafting || !HasStateChangeTimer())
        return;

    SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Athena.Crafting.CraftingCancelled")), false), 0.0f);
    HandleCratingObjectResetting();
    SetCraftingObjectState(ECraftingObjectState::Resetting);
}

void UCraftingObjectComponent::ClaimCraftingResults()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    HandleCratingObjectResetting();
    SetCraftingObjectState(ECraftingObjectState::Resetting);
}

void UCraftingObjectComponent::EjectItems()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

    auto OwnerInterface = Owner->GetInterfaceAddress(FortInventoryOwnerInterface);
    auto OwnerController = Cast<AFortPlayerController>(Owner);
    auto OwnerInventoryProperty = OwnerController ? nullptr : Owner->GetClass()->FindPropertyByName(FName(TEXT("Inventory")));
    auto OwnerInventory = OwnerController ? OwnerController->GetWorldInventory() : OwnerInventoryProperty ? *OwnerInventoryProperty->ContainerPtrToValuePtr<AFortInventory*>(Owner) : nullptr;

    if (!OwnerInterface || !OwnerInventory)
        return;

    TScriptInterface<IFortInventoryOwnerInterface> OwnerInventoryOwner;
    OwnerInventoryOwner.SetObject(Owner);
    OwnerInventoryOwner.SetInterface(OwnerInterface);

    auto OwnerPawn = OwnerController ? (AActor*)OwnerController->GetPawn() : nullptr;
    auto SpawnSource = OwnerPawn ? OwnerPawn : Owner;
    const TArray<UFortWorldItem*> Items = OwnerInventory->GetInventory().GetItemInstances();

    for (auto Item : Items)
    {
        if (!Item)
            continue;

        auto& ItemEntry = Item->GetItemEntry();
        const int32 Count = ItemEntry.GetCount();

        UFortKismetLibrary::K2_SpawnPickupInWorld(SpawnSource, Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition()), Count, SpawnSource->GetActorLocation(), SpawnSource->GetActorRotation().Vector(), 0, true);
        UInventoryManagementLibrary::RemoveItem(OwnerInventoryOwner, ItemEntry.GetItemGuid(), Count);
    }
}

void UCraftingObjectComponent::GiveCraftingResults()
{
    DETOUR(UCraftingObjectComponent, GiveCraftingResults, Anchor::Hook()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, 0x5; mov rcx, rbx; call 0x*")
        .Call()
        .Call()
        .Optional()
        .Inline());

    auto Owner = GetOwner();

    if (Owner && Owner->HasAuthority())
    {
        static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

        auto Instigator = GetCraftingInstigator();
        auto OwnerController = Cast<AFortPlayerController>(Owner);
        auto OwnerPawn = OwnerController ? (AActor*)OwnerController->GetPawn() : nullptr;
        auto SpawnSource = OwnerPawn ? OwnerPawn : Owner;
        auto OwnerInterface = Owner->GetInterfaceAddress(FortInventoryOwnerInterface);
        auto InstigatorInterface = Instigator ? Instigator->GetInterfaceAddress(FortInventoryOwnerInterface) : nullptr;

        TScriptInterface<IFortInventoryOwnerInterface> OwnerInventoryOwner;
        OwnerInventoryOwner.SetObject(Owner);
        OwnerInventoryOwner.SetInterface(OwnerInterface);

        TScriptInterface<IFortInventoryOwnerInterface> InstigatorInventoryOwner;
        InstigatorInventoryOwner.SetObject(Instigator);
        InstigatorInventoryOwner.SetInterface(InstigatorInterface);

        for (auto& Result : GetCraftingResults())
        {
            auto ItemDefinition = Cast<UFortWorldItemDefinition>(Result.Item);

            if (!ItemDefinition)
                continue;

            if (GetGiveToCraftingObject() && OwnerInterface)
            {
                UFortKismetLibrary::GiveItemToInventoryOwner(OwnerInventoryOwner, ItemDefinition, Result.Count, true);
                continue;
            }

            if (GetGiveResultToInstigator() && InstigatorInterface)
            {
                UFortKismetLibrary::GiveItemToInventoryOwner(InstigatorInventoryOwner, ItemDefinition, Result.Count, true);
                continue;
            }

            UFortKismetLibrary::K2_SpawnPickupInWorld(SpawnSource, ItemDefinition, Result.Count, SpawnSource->GetActorLocation(), SpawnSource->GetActorRotation().Vector(), 0, true);
        }
    }

    Original();
}

void UCraftingObjectComponent::GiveCraftingIngredients()
{
    DETOUR(UCraftingObjectComponent, GiveCraftingIngredients, Anchor::Hook()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, 0x5; mov rcx, rbx; call 0x*")
        .Call()
        .Call(2)
        .Optional()
        .Inline());

    auto Owner = GetOwner();

    if (Owner && Owner->HasAuthority())
    {
        static auto FortInventoryOwnerInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

        auto Instigator = GetCraftingInstigator();
        auto OwnerController = Cast<AFortPlayerController>(Owner);
        auto OwnerPawn = OwnerController ? (AActor*)OwnerController->GetPawn() : nullptr;
        auto SpawnSource = OwnerPawn ? OwnerPawn : Owner;
        auto OwnerInventory = OwnerController ? OwnerController->GetWorldInventory() : nullptr;
        auto InstigatorInventory = Instigator ? Instigator->GetWorldInventory() : nullptr;
        auto OwnerInterface = Owner->GetInterfaceAddress(FortInventoryOwnerInterface);
        auto InstigatorInterface = Instigator ? Instigator->GetInterfaceAddress(FortInventoryOwnerInterface) : nullptr;

        TScriptInterface<IFortInventoryOwnerInterface> OwnerInventoryOwner;
        OwnerInventoryOwner.SetObject(Owner);
        OwnerInventoryOwner.SetInterface(OwnerInterface);

        TScriptInterface<IFortInventoryOwnerInterface> InstigatorInventoryOwner;
        InstigatorInventoryOwner.SetObject(Instigator);
        InstigatorInventoryOwner.SetInterface(InstigatorInterface);

        for (auto& ItemEntry : GetIngredientItems())
        {
            if (GetGiveIngredientsToCraftingObject() && OwnerInterface)
            {
                if (OwnerInventory && OwnerInventory->GetOverflowFromAddingItem(ItemEntry) <= 0)
                {
                    UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(OwnerInventoryOwner, ItemEntry);
                    continue;
                }
            }
            else if (GetGiveIngredientsToInstigator() && InstigatorInterface && InstigatorInventory)
            {
                if (InstigatorInventory->GetOverflowFromAddingItem(ItemEntry) <= 0)
                {
                    UInventoryManagementLibrary::GiveItemEntryToInventoryOwner(InstigatorInventoryOwner, ItemEntry);
                    continue;
                }
            }

            auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition());

            if (ItemDefinition)
                UFortKismetLibrary::K2_SpawnPickupInWorld(SpawnSource, ItemDefinition, ItemEntry.GetCount(), SpawnSource->GetActorLocation() + GetIngredientSpawnOffset(), SpawnSource->GetActorRotation().Vector(), 0, true);
        }
    }

    Original();
}

void UCraftingObjectComponent::HandleCratingObjectResetting()
{
    static auto HandleCratingObjectResetting = Anchor::Function<void (*)(UCraftingObjectComponent*)>(TEXT("UCraftingObjectComponent::HandleCratingObjectResetting"))
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, 0x5; mov rcx, rbx; call 0x*")
        .Call()
        .Get();

    HandleCratingObjectResetting(this);
}

void UCraftingObjectComponent::SetCraftingObjectState(ECraftingObjectState InCraftingObjectState)
{
    static auto SetCraftingObjectState = Anchor::Function<void (*)(UCraftingObjectComponent*, ECraftingObjectState)>(TEXT("UCraftingObjectComponent::SetCraftingObjectState"))
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, 0x5; mov rcx, rbx; call 0x*")
        .Call(2)
        .Get();

    SetCraftingObjectState(this, InCraftingObjectState);
}

int32 UCraftingObjectComponent::SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude)
{
    static auto SendGameplayEvent = Anchor::Function<int32 (*)(UCraftingObjectComponent*, FGameplayTag, float)>(TEXT("UCraftingObjectComponent::SendGameplayEvent"))
        .String("Overcrafted")
        .Caller()
        .Find("xorps xmm2, xmm2; mov rcx, rbx; mov rdx, [rax]; call 0x*")
        .Call()
        .Get();

    return SendGameplayEvent(this, EventTag, EventMagnitude);
}

ECraftingObjectState UCraftingObjectComponent::GetCraftingObjectState() const
{
    static auto CraftingObjectRepStateData = StaticClass()->FindPropertyByName(FName(TEXT("CraftingObjectRepStateData")));

    return *CraftingObjectRepStateData->ContainerPtrToValuePtr<ECraftingObjectState>(this);
}

AFortPlayerController* UCraftingObjectComponent::GetCraftingInstigator() const
{
    static auto CraftingInstigator = StaticClass()->FindPropertyByName(FName(TEXT("CraftingInstigator")));

    return CraftingInstigator->ContainerPtrToValuePtr<TWeakObjectPtr<AFortPlayerController>>(this)->Get();
}

TArray<FItemAndCount>& UCraftingObjectComponent::GetCraftingResults()
{
    static auto CraftingResults = StaticClass()->FindPropertyByName(FName(TEXT("CraftingResults")));

    return *CraftingResults->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}

TArray<FFortItemEntry>& UCraftingObjectComponent::GetIngredientItems()
{
    static auto IngredientItems = StaticClass()->FindPropertyByName(FName(TEXT("IngredientItems")));

    return *IngredientItems->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

const FVector& UCraftingObjectComponent::GetIngredientSpawnOffset() const
{
    static auto IngredientSpawnOffset = StaticClass()->FindPropertyByName(FName(TEXT("IngredientSpawnOffset")));

    return *IngredientSpawnOffset->ContainerPtrToValuePtr<FVector>(this);
}

bool UCraftingObjectComponent::GetGiveIngredientsToCraftingObject() const
{
    static auto bGiveIngredientsToCraftingObject = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGiveIngredientsToCraftingObject")));

    return bGiveIngredientsToCraftingObject->GetPropertyValue_InContainer(this);
}

bool UCraftingObjectComponent::GetGiveIngredientsToInstigator() const
{
    static auto bGiveIngredientsToInstigator = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGiveIngredientsToInstigator")));

    return bGiveIngredientsToInstigator->GetPropertyValue_InContainer(this);
}

bool UCraftingObjectComponent::GetGiveToCraftingObject() const
{
    static auto bGiveToCraftingObject = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGiveToCraftingObject")));

    return bGiveToCraftingObject->GetPropertyValue_InContainer(this);
}

bool UCraftingObjectComponent::GetGiveResultToInstigator() const
{
    static auto bGiveResultToInstigator = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bGiveResultToInstigator")));

    return bGiveResultToInstigator->GetPropertyValue_InContainer(this);
}

bool UCraftingObjectComponent::HasStateChangeTimer() const
{
    static auto StateChangeTimerHandle = Anchor::Offset(TEXT("UCraftingObjectComponent::StateChangeTimerHandle"))
        .String("Overcrafted")
        .Caller()
        .FindEither("lea rdi, [rcx+0x*]", "mov [rcx+0x*], rdi")
        .Disp();

    return StateChangeTimerHandle > 0 && *(uint64*)((uint8*)this + StateChangeTimerHandle) != 0;
}
