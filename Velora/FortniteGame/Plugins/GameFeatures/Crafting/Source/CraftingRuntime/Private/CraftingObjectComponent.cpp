#include "pch.h"
#include "CraftingObjectComponent.h"
#include "Containers/Set.h"
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

template<>
UEnum* StaticEnum<ECraftingObjectState>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.ECraftingObjectState"));

    return Enum;
}

void UCraftingObjectComponent::StartCrafting(AFortPlayerController* Instigator, const FName& FormulaRow)
{
    static auto StartCrafting = Anchor::Function<void (*)(UCraftingObjectComponent*, AFortPlayerController*, const FName*)>(TEXT("UCraftingObjectComponent::StartCrafting"))
        .Exec(TEXT("/Script/CraftingRuntime.CraftingObjectComponent:HandlePickupCraftingItemPickedUp"))
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("mov r8, rsi; mov rdx, rbx; mov rcx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Smallest(TEXT("Event.Athena.Crafting.CraftingFailed"))
        .Start()
        .Get();

    StartCrafting(this, Instigator, &FormulaRow);
}

void UCraftingObjectComponent::StartCrafting(AFortPlayerController* Instigator, const FName& FormulaRow, int32 NumberToCraft, FCraftingMultiKey Key, float OverriddenObjectStateLength)
{
    static auto StartCrafting = Anchor::Function<void (*)(UCraftingObjectComponent*, AFortPlayerController*, const FName*, int32, FCraftingMultiKey, float)>(TEXT("UCraftingObjectComponent::StartCrafting multi key"))
        .Exec(TEXT("/Script/CraftingRuntime.CraftingObjectComponent:HandlePickupCraftingItemPickedUp"))
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("mov rbx, [rbx]; call 0x*")
        .Find("mov [rsp+0x20], rbx")
        .Find("call 0x*")
        .Target()
        .Get();

    StartCrafting(this, Instigator, &FormulaRow, NumberToCraft, Key, OverriddenObjectStateLength);
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

void UCraftingObjectComponent::PickupItemAndStartCrafting(AFortPlayerController* Instigator, AFortPickup* Pickup, const FName& FormulaRow, FCraftingMultiKey Key)
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || !Pickup)
        return;

    static auto FindIdByHash = Anchor::Optional<int32 (*)(void*, uint32, const FCraftingMultiKey*)>(TEXT("UCraftingObjectComponent::CraftingObjectServerStateData FindIdByHash"))
        .String("Overcrafted")
        .Start()
        .Call(1)
        .Is("mov eax, [rcx+0x8]; mov r10, rcx; cmp eax, [rcx+0x34]")
        .Get();

    static auto EmplaceByHash = Anchor::Optional<void (*)(void*, int32*, uint32, const FCraftingMultiKey**, bool*)>(TEXT("UCraftingObjectComponent::CraftingObjectServerStateData EmplaceByHash"))
        .String("Overcrafted")
        .Start()
        .Find("lea r9, [rbp+0x*]")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto CraftingObjectServerStateData = StaticClass()->FindPropertyByName(FName(TEXT("CraftingObjectServerStateData")));
    static auto KeyStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.CraftingMultiKey"));
    static auto ServerStateStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.CraftingObjectServerStateData"));
    static auto PendingPickupCraftingItem = ServerStateStruct ? ServerStateStruct->FindPropertyByName(FName(TEXT("PendingPickupCraftingItem"))) : nullptr;
    static auto PendingPickupCraftingFormula = ServerStateStruct ? ServerStateStruct->FindPropertyByName(FName(TEXT("PendingPickupCraftingFormula"))) : nullptr;
    static auto OnPickup = Pickup->GetClass()->FindPropertyByName(FName(TEXT("OnPickup")));

    if (!FindIdByHash || !EmplaceByHash || !CraftingObjectServerStateData || !KeyStruct || !PendingPickupCraftingItem || !PendingPickupCraftingFormula || !OnPickup)
        return;

    auto ServerStateMap = CraftingObjectServerStateData->ContainerPtrToValuePtr<uint8>(this);
    auto KeyHash = (uint32)Key.Key;
    auto ElementId = FindIdByHash(ServerStateMap, KeyHash, &Key);

    if (ElementId == -1)
    {
        const FCraftingMultiKey* KeyInitializer = &Key;
        EmplaceByHash(ServerStateMap, &ElementId, KeyHash, &KeyInitializer, nullptr);
    }

    if (ElementId < 0)
        return;

    const int32 KeyAlignment = KeyStruct->GetMinAlignment();
    const int32 ValueAlignment = ServerStateStruct->GetMinAlignment();
    const int32 PairAlignment = ValueAlignment > KeyAlignment ? ValueAlignment : KeyAlignment;
    const int32 ElementAlignment = PairAlignment > (int32)alignof(int32) ? PairAlignment : (int32)alignof(int32);
    const int32 ValueOffset = (KeyStruct->GetPropertiesSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    const int32 PairSize = (ValueOffset + ServerStateStruct->GetPropertiesSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    const int32 ElementSize = (PairSize + (int32)sizeof(FSetElementId) + (int32)sizeof(int32) + ElementAlignment - 1) & ~(ElementAlignment - 1);
    auto ServerState = *(uint8**)ServerStateMap + ElementId * ElementSize + ValueOffset;

    *PendingPickupCraftingItem->ContainerPtrToValuePtr<AFortPickup*>(ServerState) = Pickup;
    *PendingPickupCraftingFormula->ContainerPtrToValuePtr<FName>(ServerState) = FormulaRow;

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("HandlePickupCraftingItemPickedUp")));

    OnPickup->ContainerPtrToValuePtr<FMulticastScriptDelegate>(Pickup)->AddUnique(Delegate);
}

void UCraftingObjectComponent::CancelCrafting()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority() || GetCraftingObjectState() != ECraftingObjectState::Crafting || !HasStateChangeTimer())
        return;

    static auto Resetting = (ECraftingObjectState)StaticEnum<ECraftingObjectState>()->GetValueByName(FName(TEXT("ECraftingObjectState::Resetting")));

    SendGameplayEvent(FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Athena.Crafting.CraftingCancelled")), false), 0.0f);
    HandleCratingObjectResetting();
    SetCraftingObjectState(Resetting);
}

void UCraftingObjectComponent::CancelCrafting(FCraftingMultiKey Key)
{
    static auto CancelCrafting = Anchor::Function<void (*)(UCraftingObjectComponent*, FCraftingMultiKey)>(TEXT("UCraftingObjectComponent::CancelCrafting multi key"))
        .String("Event.Athena.Crafting.CraftingCancelled")
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("mov r8d, [*]")
        .Start()
        .Get();

    CancelCrafting(this, Key);
}

void UCraftingObjectComponent::ClaimCraftingResults()
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    static auto Resetting = (ECraftingObjectState)StaticEnum<ECraftingObjectState>()->GetValueByName(FName(TEXT("ECraftingObjectState::Resetting")));

    HandleCratingObjectResetting();
    SetCraftingObjectState(Resetting);
}

void UCraftingObjectComponent::ClaimCraftingResults(FCraftingMultiKey Key)
{
    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    static auto Resetting = (ECraftingObjectState)StaticEnum<ECraftingObjectState>()->GetValueByName(FName(TEXT("ECraftingObjectState::Resetting")));

    HandleCratingObjectResetting(Key);
    SetCraftingObjectState(Key, Resetting);
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
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, *; xorps xmm2, xmm2; mov rcx, rbx; call 0x*")
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
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, *; xorps xmm2, xmm2; mov rcx, rbx; call 0x*")
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

        auto& IngredientItems = GetIngredientItems();
        auto NonConsumedIngredientItemIndices = GetNonConsumedIngredientItemIndices();
        auto& PendingPickupCraftingItemEntry = GetPendingPickupCraftingItemEntry();

        if (PendingPickupCraftingItemEntry.GetItemDefinition())
        {
            const int32 PendingIndex = IngredientItems.AddUninitialized();

            new (&IngredientItems[PendingIndex]) FFortItemEntry(PendingPickupCraftingItemEntry);

            if (NonConsumedIngredientItemIndices)
                NonConsumedIngredientItemIndices->Add(PendingIndex);

            FFortItemEntry::StaticStruct()->DestroyStruct(&PendingPickupCraftingItemEntry);
            FFortItemEntry::StaticStruct()->InitializeStruct(&PendingPickupCraftingItemEntry);
            GetPendingPickupHeldCount() = 0;
        }

        const int32 NumIngredients = NonConsumedIngredientItemIndices ? NonConsumedIngredientItemIndices->Num() : IngredientItems.Num();

        for (int32 Ingredient = 0; Ingredient < NumIngredients; Ingredient++)
        {
            const int32 Index = NonConsumedIngredientItemIndices ? (*NonConsumedIngredientItemIndices)[Ingredient] : Ingredient;

            if (Index < 0 || Index >= IngredientItems.Num())
                continue;

            auto& ItemEntry = IngredientItems[Index];

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
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, *; xorps xmm2, xmm2; mov rcx, rbx; call 0x*")
        .Call()
        .Get();

    HandleCratingObjectResetting(this);
}

void UCraftingObjectComponent::HandleCratingObjectResetting(FCraftingMultiKey Key)
{
    static auto HandleCratingObjectResetting = Anchor::Function<void (*)(UCraftingObjectComponent*, FCraftingMultiKey)>(TEXT("UCraftingObjectComponent::HandleCratingObjectResetting multi key"))
        .String("Event.Athena.Crafting.CraftingCancelled")
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("mov r8d, [*]")
        .Start()
        .Find("mov r8b, 0x7")
        .Last("call 0x*")
        .Target()
        .Get();

    HandleCratingObjectResetting(this, Key);
}

void UCraftingObjectComponent::SetCraftingObjectState(ECraftingObjectState InCraftingObjectState)
{
    static auto SetCraftingObjectState = Anchor::Function<void (*)(UCraftingObjectComponent*, ECraftingObjectState)>(TEXT("UCraftingObjectComponent::SetCraftingObjectState"))
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, 0x5; mov rcx, rbx; call 0x*")
        .Call(2)
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("mov rcx, rbx; call 0x*; mov dl, *; xorps xmm2, xmm2; mov rcx, rbx; call 0x*")
        .Call(2)
        .Get();

    SetCraftingObjectState(this, InCraftingObjectState);
}

void UCraftingObjectComponent::SetCraftingObjectState(FCraftingMultiKey Key, ECraftingObjectState InCraftingObjectState)
{
    static auto SetCraftingObjectState = Anchor::Function<void (*)(UCraftingObjectComponent*, const FCraftingMultiKey*, ECraftingObjectState, float, float)>(TEXT("UCraftingObjectComponent::SetCraftingObjectState multi key"))
        .String("Event.Athena.Crafting.CraftingCancelled")
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("mov r8d, [*]")
        .Start()
        .Find("mov r8b, 0x7")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto OverriddenObjectStateLength = Anchor::Optional<const float*>(TEXT("UCraftingObjectComponent::SetCraftingObjectState overridden object state length"))
        .String("Event.Athena.Crafting.CraftingCancelled")
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("mov r8d, [*]")
        .Start()
        .Find("mov r8b, 0x7")
        .Last("movss xmm0, dword ptr [*]")
        .Target()
        .Get();

    SetCraftingObjectState(this, &Key, InCraftingObjectState, 0.0f, OverriddenObjectStateLength ? *OverriddenObjectStateLength : 0.0f);
}

int32 UCraftingObjectComponent::SendGameplayEvent(FGameplayTag EventTag, float EventMagnitude)
{
    static auto SendGameplayEvent = Anchor::Function<int32 (*)(UCraftingObjectComponent*, FGameplayTag, float)>(TEXT("UCraftingObjectComponent::SendGameplayEvent"))
        .String("Overcrafted")
        .Caller()
        .Find("xorps xmm2, xmm2; mov rcx, rbx; mov ?dx, [rax]; call 0x*")
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
    static auto AllOfTheIngredientItems = StaticClass()->FindPropertyByName(FName(TEXT("AllOfTheIngredientItems")));

    return *(IngredientItems ? IngredientItems : AllOfTheIngredientItems)->ContainerPtrToValuePtr<TArray<FFortItemEntry>>(this);
}

TArray<int32>* UCraftingObjectComponent::GetNonConsumedIngredientItemIndices()
{
    static auto NonConsumedIngredientItemIndices = StaticClass()->FindPropertyByName(FName(TEXT("NonConsumedIngredientItemIndices")));

    return NonConsumedIngredientItemIndices ? NonConsumedIngredientItemIndices->ContainerPtrToValuePtr<TArray<int32>>(this) : nullptr;
}

FFortItemEntry& UCraftingObjectComponent::GetPendingPickupCraftingItemEntry()
{
    static auto PendingPickupCraftingItemEntry = StaticClass()->FindPropertyByName(FName(TEXT("PendingPickupCraftingItemEntry")));

    return *PendingPickupCraftingItemEntry->ContainerPtrToValuePtr<FFortItemEntry>(this);
}

int32& UCraftingObjectComponent::GetPendingPickupHeldCount()
{
    static auto PendingPickupHeldCount = StaticClass()->FindPropertyByName(FName(TEXT("PendingPickupHeldCount")));

    return *PendingPickupHeldCount->ContainerPtrToValuePtr<int32>(this);
}

FVector UCraftingObjectComponent::GetIngredientSpawnOffset() const
{
    static auto IngredientSpawnOffset = StaticClass()->FindPropertyByName(FName(TEXT("IngredientSpawnOffset")));

    return IngredientSpawnOffset->GetPropertyValue_InContainer<FVector>(this);
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
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("lea rdi, [rbx+0x*]; mov rcx, rax; mov rdx, rdi; mov rsi, rax; call 0x*")
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("mov rbx, rcx; call 0x*; test al, al; jz 0x*; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("add rbx, 0x*")
        .Or()
        .String("Overcrafted")
        .Caller()
        .Find("lea rdi, [rbx+0x*]; call 0x*; mov rcx, rax; call 0x*; mov rcx, rax; mov rdx, rdi")
        .Get();

    return StateChangeTimerHandle > 0 && *(uint64*)((uint8*)this + StateChangeTimerHandle) != 0;
}
