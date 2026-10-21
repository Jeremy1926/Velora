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
#include "Weapons/FortWeapon.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

struct FPendingSwap
{
    TWeakObjectPtr<AFortPickup> Pickup;
    FGuid SwapItemGuid;
};

static TArray<FPendingSwap> PendingSwaps;

UClass* AFortPickup::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPickup"));

    return Class;
}

AFortPickup* AFortPickup::CreateFromData(UWorld* World, const FFortItemEntry& ItemEntry, const FVector& SpawnLocation, const FRotator& SpawnRotation, AFortPlayerController* OptionalOwnerPC, UClass* OverrideClass, AActor* Owner, EFortPickupSourceTypeFlag SourceTypeFlags, bool bRandomRotation, bool bPickupOnlyRelevantToOwner)
{
    static auto bArguments = Anchor::Exists()
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("mov qword ptr [rsp+0x28], *")
        .Get();

    static auto bItemEntryByValue = Anchor::Exists()
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("lea r8d, [r9+0x1]; call 0x*; mov rdx, rax")
        .Get();

    static auto CreateFromData = Anchor::Function<const void*>(TEXT("AFortPickup::CreateFromData"))
        .If(bArguments)
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("mov qword ptr [rsp+0x28], *")
        .Call()
        .Or()
        .If(!bArguments && bItemEntryByValue)
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("lea r8d, [r9+0x1]")
        .Call(3)
        .Or()
        .If(!bArguments && !bItemEntryByValue)
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("lea r8d, [r9+0x1]")
        .Call(2)
        .Get();
    static auto PickupSourceTypeFlags = StaticClass()->FindPropertyByName(FName(TEXT("PickupSourceTypeFlags")));

    if (bItemEntryByValue)
    {
        struct
        {
            FVector SpawnLocation;
            FRotator SpawnRotation;
            AFortPlayerController* OptionalOwnerPC;
            UClass* OverrideClass;
            AActor* Owner;
            EFortPickupSourceTypeFlag SourceTypeFlags;
            EFortPickupSpawnSource PickupSpawnSource;
            uint8 bRandomRotation : 1;
            uint8 bPickupOnlyRelevantToOwner : 1;
        } const AfterItemEntry{ SpawnLocation, SpawnRotation, OptionalOwnerPC, OverrideClass, Owner, SourceTypeFlags, EFortPickupSpawnSource::Unset, bRandomRotation, bPickupOnlyRelevantToOwner };

        TArray<uint8> CreationData;
        CreationData.AddUninitialized(sizeof(UWorld*) + FFortItemEntry::GetTypeSize() + sizeof(AfterItemEntry));

        *(UWorld**)CreationData.GetData() = World;
        auto ItemEntryCopy = new(CreationData.GetData() + sizeof(UWorld*)) FFortItemEntry(ItemEntry);
        memcpy(CreationData.GetData() + sizeof(UWorld*) + FFortItemEntry::GetTypeSize(), &AfterItemEntry, sizeof(AfterItemEntry));

        auto Pickup = ((AFortPickup* (*)(const uint8*))CreateFromData)(CreationData.GetData());
        ItemEntryCopy->~FFortItemEntry();

        return Pickup;
    }

    if (!bArguments && PickupSourceTypeFlags && PickupSourceTypeFlags->GetSize() == sizeof(EFortPickupSourceTypeFlag))
    {
        struct FortPickupCreationData
        {
            UWorld* World;
            const FFortItemEntry* ItemEntry;
            const FVector* SpawnLocation;
            const FRotator* SpawnRotation;
            AFortPlayerController* OptionalOwnerPC;
            UClass* OverrideClass;
            AActor* Owner;
            EFortPickupSourceTypeFlag SourceTypeFlags;
            EFortPickupSpawnSource PickupSpawnSource;
            uint8 bRandomRotation : 1;
            uint8 bPickupOnlyRelevantToOwner : 1;
        };

        return ((AFortPickup* (*)(const FortPickupCreationData&))CreateFromData)({ World, &ItemEntry, &SpawnLocation, &SpawnRotation, OptionalOwnerPC, OverrideClass, Owner, SourceTypeFlags, EFortPickupSpawnSource::Unset, bRandomRotation, bPickupOnlyRelevantToOwner });
    }

    if (!bArguments)
        return ((AFortPickup* (*)(const FortPickupCreationData&))CreateFromData)({ World, &ItemEntry, &SpawnLocation, &SpawnRotation, OptionalOwnerPC, OverrideClass, Owner, bRandomRotation, (uint32)SourceTypeFlags });

    return ((AFortPickup* (*)(UWorld*, const FFortItemEntry&, const FVector&, const FRotator&, AFortPlayerController*, UClass*, AActor*, bool, bool))CreateFromData)(World, ItemEntry, SpawnLocation, SpawnRotation, OptionalOwnerPC, OverrideClass, Owner, bRandomRotation, bPickupOnlyRelevantToOwner);
}

void AFortPickup::TossPickup(const FVector& FinalLocation, AFortPawn* ItemOwner, int32 OverrideMaxStackCount, bool bToss, bool bShouldCombinePickupsWhenTossCompletes, EFortPickupSourceTypeFlag InPickupSourceTypeFlags, EFortPickupSpawnSource InPickupSpawnSource)
{
    static auto TossPickup = Anchor::Function<const void*>(TEXT("AFortPickup::TossPickup"))
        .Native(TEXT("/Script/FortniteGame.FortPickup:TossPickup"))
        .Get();

    static auto bTakesCombine = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPickup:TossPickup")))->FindPropertyByName(FName(TEXT("bShouldCombinePickupsWhenTossCompletes"))) != nullptr;

    if (bTakesCombine)
    {
        ((void (*)(AFortPickup*, FVector, AFortPawn*, int32, bool, bool, EFortPickupSourceTypeFlag, EFortPickupSpawnSource))TossPickup)(this, FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, bShouldCombinePickupsWhenTossCompletes, InPickupSourceTypeFlags, InPickupSpawnSource);
        return;
    }

    ((void (*)(AFortPickup*, FVector, AFortPawn*, int32, bool, EFortPickupSourceTypeFlag, EFortPickupSpawnSource))TossPickup)(this, FinalLocation, ItemOwner, OverrideMaxStackCount, bToss, InPickupSourceTypeFlags, InPickupSpawnSource);
}

decltype(AFortPickup::GameGivePickupTo) AFortPickup::GameGivePickupTo = nullptr;

void AFortPickup::GivePickupTo(IFortInventoryOwnerInterface* InventoryOwner, bool DestoryAfterPickup)
{
    static auto bStripped = Anchor::Exists()
        .At(std::bit_cast<const uint8_t*>(GameGivePickupTo))
        .Is("ret*")
        .Get();

    if (HasAuthority() && InventoryOwner)
    {
        GivePickupItemsToInventoryOwner(InventoryOwner);

        if (bStripped)
            Destroy();
    }

    if (!bStripped)
        (this->*GameGivePickupTo)(InventoryOwner, DestoryAfterPickup);
}

void AFortPickup::SetSwapItemGuid(const FGuid& SwapItemGuid)
{
    PendingSwaps.Add(FPendingSwap{ this, SwapItemGuid });
}

void AFortPickup::GivePickupItemsToInventoryOwner(IFortInventoryOwnerInterface* InventoryOwner)
{
    FGuid SwapItemGuid;

    for (int32 Index = PendingSwaps.Num() - 1; Index >= 0; Index--)
    {
        auto Pickup = PendingSwaps[Index].Pickup.Get();

        if (Pickup && Pickup != this)
            continue;

        if (Pickup)
            SwapItemGuid = PendingSwaps[Index].SwapItemGuid;

        PendingSwaps.RemoveAt(Index);
    }

    auto PickupTarget = GetPickupLocationData().PickupTarget;
    auto PlayerController = PickupTarget ? Cast<AFortPlayerControllerAthena>(PickupTarget->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return;

    auto GiveItemEntry = [&](FFortItemEntry& PickupItemEntry)
    {
        auto ItemDefinition = PickupItemEntry.GetItemDefinition();

        if (!ItemDefinition)
            return;

        auto& Inventory = WorldInventory->GetInventory();
        const int32 MaxStackSize = ItemDefinition->GetMaxStackSize(PickupTarget->GetAbilitySystemComponent());
        int32 NumberToGive = PickupItemEntry.GetCount();
        auto bCanAddStack = true;

        static auto DecoItemDefinitionClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoItemDefinition"));
        static auto CanItemTypeBeSwapped = Anchor::Optional<bool (*)(AFortPickup*, EFortItemType, AFortPlayerController*)>(TEXT("AFortPickup::CanItemTypeBeSwapped"))
            .String(TEXT("SwapEquippedText"))
            .Find("mov r8, rax; mov rcx, *; call 0x*")
            .Call()
            .Has("bt *")
            .Get();
        auto bNeedsPrimarySlot = ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) && !ItemDefinition->IsA(DecoItemDefinitionClass);
        auto bCanBeSwapped = CanItemTypeBeSwapped ? CanItemTypeBeSwapped(this, ItemDefinition->GetItemType(), PlayerController) : bNeedsPrimarySlot;
        auto bSwapEquipped = bCanBeSwapped && !PlayerController->CanServerTryAddToQuickBar() && WorldInventory->GetOverflowFromAddingItem(PickupItemEntry) > 0;

        for (auto& ItemEntry : Inventory.GetReplicatedEntries())
        {
            if (ItemEntry.GetItemDefinition() != ItemDefinition)
                continue;

            const int32 NumberToStack = FMath::Min(NumberToGive, MaxStackSize - ItemEntry.GetCount());

            if (NumberToStack > 0)
            {
                int32 NewItemCount = 0;
                int32 ShouldShowItemToast = 0;

                ItemEntry.GetStateValue(EFortItemEntryState::NewItemCount, NewItemCount);

                ItemEntry.SetCount(ItemEntry.GetCount() + NumberToStack);
                ItemEntry.SetStateValue(EFortItemEntryState::NewItemCount, NewItemCount + NumberToStack);

                if (PickupItemEntry.GetStateValue(EFortItemEntryState::ShouldShowItemToast, ShouldShowItemToast))
                    ItemEntry.SetStateValue(EFortItemEntryState::ShouldShowItemToast, ShouldShowItemToast);

                Inventory.MarkItemDirty(ItemEntry);

                NumberToGive -= NumberToStack;
            }

            bCanAddStack = ItemDefinition->DoesAllowMultipleStacks();
        }

        WorldInventory->HandleInventoryLocalUpdate();

        auto QuickBars = PlayerController->GetQuickBars();

        auto FindItem = [&](const FGuid& ItemGuid) -> UFortItem*
        {
            for (auto Item : Inventory.GetItemInstances())
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

        if (bSwapEquipped && SwapItemGuid == FGuid() && PickupTarget->GetCurrentWeapon())
            SwapItemGuid = PickupTarget->GetCurrentWeapon()->GetItemEntryGuid();

        auto bEquipNewItem = bSwapEquipped && PickupTarget->GetCurrentWeapon() && PickupTarget->GetCurrentWeapon()->GetItemEntryGuid() == SwapItemGuid;

        if (bCanAddStack && NumberToGive > 0 && SwapItemGuid != FGuid())
        {
            int32 SwapCount = 0;

            for (auto& Entry : Inventory.GetReplicatedEntries())
            {
                if (Entry.GetItemGuid() == SwapItemGuid)
                    SwapCount = Entry.GetCount();
            }

            if (SwapCount > 0)
                PlayerController->ServerSpawnInventoryDrop_Implementation(SwapItemGuid, SwapCount);

            SwapItemGuid = FGuid();
        }

        if (bSwapEquipped && WorldInventory->GetOverflowFromAddingItem(PickupItemEntry) > 0)
            bCanAddStack = false;

        while (bCanAddStack && NumberToGive > 0)
        {
            auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(ItemDefinition, FMath::Min(NumberToGive, MaxStackSize), PickupItemEntry.GetLevel());
            ItemEntry->GetLoadedAmmo() = PickupItemEntry.GetLoadedAmmo();
            ItemEntry->GetDurability() = PickupItemEntry.GetDurability();

            for (auto& StateValue : PickupItemEntry.GetStateValues())
                ItemEntry->SetStateValue(StateValue.GetStateType(), StateValue.GetIntValue());

            ItemEntry->SetStateValue(EFortItemEntryState::NewItemCount, ItemEntry->GetCount());

            const FGuid ItemGuid = ItemEntry->GetItemGuid();
            const int32 ItemCount = ItemEntry->GetCount();

            NumberToGive -= ItemCount;

            ItemEntry->GetParentInventory() = WorldInventory;
            ItemEntry->GetIsReplicatedCopy() = true;

            if (ItemEntry->GetParentInventory().IsValid())
                ItemEntry->GetParentInventory().Get()->SetItemRequiresUpdate(*ItemEntry);

            ItemEntry->GetIsDirty() = true;

            WorldInventory->HandleInventoryLocalUpdate();

            if (!PlayerController->CanServerTryAddToQuickBar())
            {
                if (bEquipNewItem)
                    PlayerController->ClientEquipItem(ItemGuid, true);

                bEquipNewItem = false;

                continue;
            }

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

                for (auto& Entry : Inventory.GetReplicatedEntries())
                {
                    if (Entry.GetItemGuid() == SwapItemGuid)
                        SwapCount = Entry.GetCount();
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
            FFortItemEntry LeftoverItemEntry(ItemDefinition, NumberToGive, PickupItemEntry.GetLevel());
            LeftoverItemEntry.GetLoadedAmmo() = PickupItemEntry.GetLoadedAmmo();
            LeftoverItemEntry.GetDurability() = PickupItemEntry.GetDurability();

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
    static auto VirtualSetPickupTarget = Anchor::VirtualFunction<void (*)(AFortPickup*, AFortPawn*, float, const FVector&, bool)>(TEXT("AFortPickup::SetPickupTarget"))
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("ret")
        .Last("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    if (VirtualSetPickupTarget)
        return VirtualSetPickupTarget(this, InPickupTarget, InFlyTime, InStartDirection, bPlayPickupSound);

    static auto SetPickupTarget = Anchor::Function<void (*)(AFortPickup*, AFortPawn*, float, const FVector&, bool)>(TEXT("AFortPickup::SetPickupTarget"))
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("ret")
        .Last("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    SetPickupTarget(this, InPickupTarget, InFlyTime, InStartDirection, bPlayPickupSound);
}

float AFortPickup::GetFlyTime() const
{
    static auto GetFlyTime = Anchor::Function<float (*)(const AFortPickup*)>(TEXT("AFortPickup::GetFlyTime"))
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("ret")
        .Last("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call [*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPickup"))
        .Find("comiss *")
        .Call()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("ret")
        .Last("mov byte ptr [rsp+0x20], 0x1")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Find("comiss *")
        .Call()
        .Get();

    return GetFlyTime(this);
}
