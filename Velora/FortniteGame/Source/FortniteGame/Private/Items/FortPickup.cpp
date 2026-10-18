#include "pch.h"
#include "Items/FortPickup.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Pawns/FortPawn.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortPickup::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPickup"));

    return Class;
}

AFortPickup* AFortPickup::CreateFromData(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& SpawnLocation, const FRotator& SpawnRotation, AFortPlayerController* OptionalOwnerPC, UClass* OverrideClass)
{
    static auto CreateFromData = (AFortPickup* (*)(UWorld*, const FFortItemEntry&, const FVector&, const FRotator&, AFortPlayerController*, UClass*))Scanner::Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")).Find("mov qword ptr [rsp+0x28], *").Call().Get(TEXT("AFortPickup::CreateFromData"));

    return CreateFromData(World, ItemEntry, SpawnLocation, SpawnRotation, OptionalOwnerPC, OverrideClass);
}

void AFortPickup::TossPickup(const FVector& FinalLocation, AFortPawn* ItemOwner, int32 OverrideMaxStackCount, bool bToss)
{
    static auto TossPickup = (void (*)(AFortPickup*, FVector, AFortPawn*, int32, bool))Scanner::Native(TEXT("/Script/FortniteGame.FortPickup:TossPickup")).Get(TEXT("AFortPickup::TossPickup"));

    TossPickup(this, FinalLocation, ItemOwner, OverrideMaxStackCount, bToss);
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
                int32 NewItemCount = 0;
                int32 ShouldShowItemToast = 0;

                ItemEntry.GetStateValue(EFortItemEntryState::NewItemCount, NewItemCount);

                ItemEntry.SetCount(ItemEntry.Count + NumberToStack);
                ItemEntry.SetStateValue(EFortItemEntryState::NewItemCount, NewItemCount + NumberToStack);

                if (PickupItemEntry.GetStateValue(EFortItemEntryState::ShouldShowItemToast, ShouldShowItemToast))
                    ItemEntry.SetStateValue(EFortItemEntryState::ShouldShowItemToast, ShouldShowItemToast);

                Inventory.MarkItemDirty(ItemEntry);

                NumberToGive -= NumberToStack;
            }

            bCanAddStack = ItemDefinition->DoesAllowMultipleStacks();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        static auto DecoItemDefinitionClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoItemDefinition"));
        auto bNeedsPrimarySlot = ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) && !ItemDefinition->IsA(DecoItemDefinitionClass);
        auto QuickBars = PlayerController->GetQuickBars();

        auto FindItem = [&](const FGuid& ItemGuid) -> UFortItem*
        {
            for (auto Item : Inventory.ItemInstances)
            {
                if (Item->GetItemGuid() == ItemGuid)
                    return Item;
            }

            return nullptr;
        };

        auto IsOnQuickBar = [&](const FGuid& ItemGuid)
        {
            for (auto QuickBarType : { EFortQuickBars::Primary, EFortQuickBars::Secondary })
            {
                for (auto& Slot : QuickBars->GetQuickBar(QuickBarType).Slots)
                {
                    if (Slot.Items.Contains(ItemGuid))
                        return true;
                }
            }

            return false;
        };

        while (bCanAddStack && NumberToGive > 0)
        {
            auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, FMath::Min(NumberToGive, MaxStackSize), PickupItemEntry.Level);
            ItemEntry->LoadedAmmo = PickupItemEntry.LoadedAmmo;
            ItemEntry->Durability = PickupItemEntry.Durability;

            for (auto& StateValue : PickupItemEntry.StateValues)
                ItemEntry->SetStateValue(StateValue.StateType, StateValue.IntValue);

            ItemEntry->SetStateValue(EFortItemEntryState::NewItemCount, ItemEntry->Count);

            const FGuid ItemGuid = ItemEntry->ItemGuid;
            const int32 ItemCount = ItemEntry->Count;

            NumberToGive -= ItemCount;

            Inventory.MarkItemDirty(*ItemEntry);
            WorldInventory->HandleInventoryLocalUpdate();

            if (auto Item = FindItem(ItemGuid))
                PlayerController->TryAddToQuickBar(Item);

            if (!bNeedsPrimarySlot || !QuickBars || IsOnQuickBar(ItemGuid))
                continue;

            auto& QuickBar = QuickBars->GetQuickBar(EFortQuickBars::Primary);
            auto FocusedSlot = QuickBar.CurrentFocusedSlot;

            if (FocusedSlot >= 0 && FocusedSlot < QuickBar.Slots.Num() && QuickBar.Slots[FocusedSlot].Items.Num())
            {
                const FGuid SwapItemGuid = QuickBar.Slots[FocusedSlot].Items[0];
                int32 SwapCount = 0;

                for (auto& Entry : Inventory.ReplicatedEntries)
                {
                    if (Entry.ItemGuid == SwapItemGuid)
                        SwapCount = Entry.Count;
                }

                if (SwapCount > 0)
                    PlayerController->ServerSpawnInventoryDrop_Implementation(SwapItemGuid, SwapCount);
            }

            if (auto Item = FindItem(ItemGuid))
                PlayerController->TryAddToQuickBar(Item);

            if (IsOnQuickBar(ItemGuid))
                continue;

            PlayerController->RemoveInventoryItem(ItemGuid, ItemCount, true);
            NumberToGive += ItemCount;
            break;
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
    static auto SetPickupTarget = (void (*)(AFortPickup*, AFortPawn*, float, const FVector&, bool))Scanner::Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")).Find("ret").Last("call 0x*").Call().Get(TEXT("AFortPickup::SetPickupTarget"));

    SetPickupTarget(this, InPickupTarget, InFlyTime, InStartDirection, bPlayPickupSound);
}

float AFortPickup::GetFlyTime() const
{
    static auto GetFlyTime = (float (*)(const AFortPickup*))Scanner::Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup")).Find("ret").Last("call 0x*").Call().Find("comiss *").Call().Get(TEXT("AFortPickup::GetFlyTime"));

    return GetFlyTime(this);
}
