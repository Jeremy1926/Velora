#include "pch.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/AI/FortAthenaAIBotInventoryItems.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Player/FortPlayerState.h"
#include "Weapons/FortWeapon.h"
#include "Templates/Casts.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaAIBotController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIBotController"));

    return Class;
}

UScriptStruct* FFortBotInventoryInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortBotInventoryInfo"));

    return Struct;
}

UFortItemDefinition*& FFortBotInventoryInfo::GetItemDefinition()
{
    static auto ItemDefinition = StaticStruct()->FindPropertyByName(FName(TEXT("ItemDefinition")));

    return *ItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);
}

UFortWorldItem*& FFortBotInventoryInfo::GetFortItem()
{
    static auto FortItem = StaticStruct()->FindPropertyByName(FName(TEXT("FortItem")));

    return *FortItem->ContainerPtrToValuePtr<UFortWorldItem*>(this);
}

EFortItemType* FFortBotInventoryInfo::GetItemType()
{
    static auto ItemType = Anchor::Offset(TEXT("FFortBotInventoryInfo::ItemType"))
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Origin("mov [*-0x*], *; mov byte ptr [*], 0x*")
        .Find("mov byte ptr [*], 0x*")
        .Disp();

    return ItemType > 0 ? (EFortItemType*)((uint8*)this + ItemType) : nullptr;
}

EFortWeaponType* FFortBotInventoryInfo::GetWeaponType()
{
    static auto WeaponType = Anchor::Offset(TEXT("FFortBotInventoryInfo::WeaponType"))
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Origin("mov [*-0x*], *; mov byte ptr [*], 0x*")
        .Find("mov byte ptr [*], 0x*; call 0x*; mov [*+0x*], al")
        .Find("mov [*+0x*], al")
        .Disp();

    return WeaponType > 0 ? (EFortWeaponType*)((uint8*)this + WeaponType) : nullptr;
}

bool AFortAthenaAIBotController::CreateInventory()
{
    if (GetInventory())
        return false;

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;

    GetInventory() = (AFortInventory*)GetWorld()->SpawnActor(AFortInventory::StaticClass(), nullptr, nullptr, SpawnParameters);

    if (!GetInventory())
        return false;

    GetInventory()->GetInventoryType() = EFortInventoryType::World;

    return true;
}

void AFortAthenaAIBotController::GiveInventory(const TArray<FItemAndCount>& Items)
{
    auto Inventory = GetInventory();

    if (!Inventory)
        return;

    for (auto& ItemAndCount : Items)
    {
        if (!ItemAndCount.Item || ItemAndCount.Count <= 0)
            continue;

        auto ItemEntry = new(Inventory->GetInventory().GetReplicatedEntries()) FFortItemEntry(ItemAndCount.Item, ItemAndCount.Count, ItemAndCount.Item->GetMinLevel());

        if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemAndCount.Item))
        {
            if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
                ItemEntry->SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry->GetLevel()));
        }

        ItemEntry->GetParentInventory() = Inventory;
        ItemEntry->GetIsReplicatedCopy() = true;

        Inventory->SetItemRequiresUpdate(*ItemEntry);

        ItemEntry->GetIsDirty() = true;
    }

    Inventory->HandleInventoryLocalUpdate();

    static auto bFindWeaponTypeForStats = Anchor::Exists()
        .FieldPattern("41 C7 00 FF FF FF FF ? 8B ? 48 83 B9 %o 00", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("Inventory"))) : nullptr)
        .Find("cmp [*+0x*], *; jnz *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]; cmp al, 0x*; jnz *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!bFindWeaponTypeForStats)
        return;

    for (auto& ItemAndCount : Items)
    {
        UFortWorldItem* WorldItem = nullptr;

        for (auto ItemInstance : Inventory->GetInventory().GetItemInstances())
        {
            if (ItemInstance && ItemInstance->GetItemEntry().GetItemDefinition() == ItemAndCount.Item)
            {
                WorldItem = ItemInstance;
                break;
            }
        }

        int32 SlotIndex = -1;

        while (WorldItem && WouldItemFitInBotInventory(WorldItem, SlotIndex) && SlotIndex >= 0)
        {
            auto& SlotItem = GetSlotItems(SlotIndex);
            auto ItemType = SlotItem.GetItemType();
            auto WeaponType = SlotItem.GetWeaponType();

            if (!ItemType || !WeaponType)
                return;

            auto ReplacedItem = SlotItem.GetFortItem();
            auto ItemDefinition = WorldItem->GetItemEntry().GetItemDefinition();

            SlotItem.GetItemDefinition() = ItemDefinition;
            SlotItem.GetFortItem() = WorldItem;
            *ItemType = ItemDefinition->GetItemType();
            *WeaponType = AFortWeapon::FindWeaponTypeForStats(ItemDefinition);

            WorldItem = ReplacedItem != WorldItem ? ReplacedItem : nullptr;
        }
    }
}

void AFortAthenaAIBotController::InitializeInventory(UFortAthenaAIBotInventoryItems* InStartupInventory)
{
    DETOUR(AFortAthenaAIBotController, InitializeInventory, Anchor::Hook()
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Optional()
        .Start()
        .Inline());

    CreateInventory();

    Original(InStartupInventory);

    TArray<FItemAndCount> Items;

    if (auto StartupInventory = GetStartupInventory())
    {
        for (auto ItemDefinition : StartupInventory->GetItems())
            Items.Add({ 1, ItemDefinition });
    }

    if (auto Pickaxe = GetCosmeticLoadoutBC().GetPickaxe())
        Items.Add({ 1, Pickaxe->GetWeaponDefinition() });

    static auto bAllowUnsupportedItems = Anchor::Offset(TEXT("AFortAthenaAIBotController::InitializeInventory allow unsupported items"))
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Find("mov [rcx+0x*], al")
        .Disp();

    if (bAllowUnsupportedItems)
        *((uint8*)this + bAllowUnsupportedItems) = GetbAllowUnsupportedItemsInDefaultInventory();

    GiveInventory(Items);

    if (bAllowUnsupportedItems)
        *((uint8*)this + bAllowUnsupportedItems) = false;
}

bool AFortAthenaAIBotController::WouldItemFitInBotInventory(UFortWorldItem* Item, int32& OutSlotIndex)
{
    static auto WouldItemFitInBotInventory = Anchor::Function<bool (*)(AFortAthenaAIBotController*, UFortWorldItem*, int32&)>(TEXT("AFortAthenaAIBotController::WouldItemFitInBotInventory"))
        .FieldPattern("41 C7 00 FF FF FF FF ? 8B ? 48 83 B9 %o 00", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("Inventory"))) : nullptr)
        .Start()
        .Get();

    return WouldItemFitInBotInventory(this, Item, OutSlotIndex);
}

void AFortAthenaAIBotController::ApplyCharacterCustomization(const FFortAthenaLoadout* InCustomizationLoadout, const UFortBotNameSettings* InBotNameSettings, bool bRequiresUniqueNetId)
{
    DETOUR(AFortAthenaAIBotController, ApplyCharacterCustomization, Anchor::Hook()
        .FieldPattern("48 83 B9 %o 00 ? 0F B6 ?", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("PlayerBotPawn"))) : nullptr)
        .Optional()
        .Start()
        .Inline());

    Original(InCustomizationLoadout, InBotNameSettings, bRequiresUniqueNetId);

    auto PlayerState = Cast<AFortPlayerState>(GetPlayerState());

    if (PlayerState && GetPawn())
        PlayerState->ApplyCharacterCustomization(GetPawn());
}

FFortBotInventoryInfo& AFortAthenaAIBotController::GetSlotItems(int32 Index)
{
    static auto SlotItems = GetClass()->FindPropertyByName(FName(TEXT("SlotItems")));

    return *SlotItems->ContainerPtrToValuePtr<FFortBotInventoryInfo>(this, Index);
}

bool AFortAthenaAIBotController::GetbAllowUnsupportedItemsInDefaultInventory() const
{
    static auto bAllowUnsupportedItemsInDefaultInventory = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bAllowUnsupportedItemsInDefaultInventory")));

    return bAllowUnsupportedItemsInDefaultInventory->GetPropertyValue_InContainer(this);
}

AFortInventory*& AFortAthenaAIBotController::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}

UFortAthenaAIBotInventoryItems* AFortAthenaAIBotController::GetStartupInventory() const
{
    static auto StartupInventory = GetClass()->FindPropertyByName(FName(TEXT("StartupInventory")));

    return *StartupInventory->ContainerPtrToValuePtr<UFortAthenaAIBotInventoryItems*>(this);
}

UBehaviorTree* AFortAthenaAIBotController::GetBehaviorTree() const
{
    static auto BehaviorTree = GetClass()->FindPropertyByName(FName(TEXT("BehaviorTree")));

    return *BehaviorTree->ContainerPtrToValuePtr<UBehaviorTree*>(this);
}

FFortAthenaLoadout& AFortAthenaAIBotController::GetCosmeticLoadoutBC()
{
    static auto CosmeticLoadoutBC = GetClass()->FindPropertyByName(FName(TEXT("CosmeticLoadoutBC")));

    return *CosmeticLoadoutBC->ContainerPtrToValuePtr<FFortAthenaLoadout>(this);
}
