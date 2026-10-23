#include "pch.h"
#include "Building/BuildingItemWeaponUpgradeActor.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortKismetLibrary.h"
#include "GameplayEffectTypes.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static FWeaponUpgradeItemRow* UpgradeItemRowBeingConsumed = nullptr;

UClass* ABuildingItemWeaponUpgradeActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingItemWeaponUpgradeActor"));

    return Class;
}

void ABuildingItemWeaponUpgradeActor::AthenaPostApplyPlaylistDataTables()
{
    DETOUR(ABuildingItemWeaponUpgradeActor, AthenaPostApplyPlaylistDataTables, Anchor::Hook()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass())
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov ?dx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::BeginPlay"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"))
        .Is("jmp 0x*")
        .Target()
        .FieldFind("cmp *[*+%x], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables"))
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"))
        .Call()
        .FieldFind("cmp *[*+%x], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables"))
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov rdx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"))
        .Is("jmp 0x*")
        .Target()
        .Find("mov *, [*+0x*]; test *, *; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .FieldFind("cmp *[*+%x], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables"))
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov rdx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"))
        .Is("jmp 0x*")
        .Target()
        .Find("test rax, rax; jz 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .FieldFind("cmp *[*+%x], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor")));

    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto WeaponUpgradeMachineSpawnChance = MapInfo ? MapInfo->GetWeaponUpgradeMachineSpawnChance() : nullptr;

    if (!HasAuthority() || !IsNetStartupActor() || !WeaponUpgradeMachineSpawnChance)
        return;

    if (WeaponUpgradeMachineSpawnChance->GetValueAtLevel(0.0f) < FMath::FRand())
        Destroy();
}

void ABuildingItemWeaponUpgradeActor::GrantOutput(AFortPlayerController* Player, UFortWorldItemDefinition* Item)
{
    DETOUR(ABuildingItemWeaponUpgradeActor, GrantOutput, Anchor::Hook()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .LastCall(Anchor::Address()
            .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer"))
            .Find("jnz 0x*")
            .Target()
            .Get())
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::GrantOutput"))
        .Or()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov rcx, *; mov *, eax; call 0x*")
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::GrantOutput"))
        .Or()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::GrantOutput"))
        .Or()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Caller()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::GrantOutput"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor")));

    auto PlayerController = Cast<AFortPlayerControllerAthena>(Player);
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!HasAuthority() || !WorldInventory || !Item)
        return;

    if (!AreConsumedItemsRemovedByGame())
    {
        auto UpgradeItemRow = UpgradeItemRowBeingConsumed;
        UpgradeItemRowBeingConsumed = nullptr;

        if (!UpgradeItemRow || UpgradeItemRow->GetUpgradedWeaponDef() != Item)
            return;

        UFortKismetLibrary::K2_RemoveItemFromPlayer(Player, (UFortWorldItemDefinition*)GetWoodItem(), GetCostForRessource(UpgradeItemRow->GetWoodCost()), false);
        UFortKismetLibrary::K2_RemoveItemFromPlayer(Player, (UFortWorldItemDefinition*)GetMetalItem(), GetCostForRessource(UpgradeItemRow->GetMetalCost()), false);
        UFortKismetLibrary::K2_RemoveItemFromPlayer(Player, (UFortWorldItemDefinition*)GetBrickItem(), GetCostForRessource(UpgradeItemRow->GetBrickCost()), false);
        UFortKismetLibrary::K2_RemoveItemFromPlayer(Player, (UFortWorldItemDefinition*)UpgradeItemRow->GetCurrentWeaponDef(), 1, false);
    }

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(Item, 1, Item->GetMinLevel());
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

    if (Item->GetItemType() == WeaponMelee || Item->GetItemType() == WeaponRanged)
    {
        ItemEntry->SetDurability(Item->GetMaxDurability(ItemEntry->GetLevel()));
        ItemEntry->SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(Item))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry->SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry->GetLevel()));
    }

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    PlayerController->ClientEquipItem(ItemGuid, true);
}

FWeaponUpgradeItemRow* ABuildingItemWeaponUpgradeActor::GetItemRow(UFortWeaponItemDefinition* WeaponDefinition, EFortWeaponUpgradeDirection Direction) const
{
    static auto StaticGetItemRow = Anchor::Optional<FWeaponUpgradeItemRow* (*)(UFortWeaponItemDefinition*, EFortWeaponUpgradeDirection)>(TEXT("ABuildingItemWeaponUpgradeActor::GetItemRow"))
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("setz dl; inc dl; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("setz dl; add dl, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("setz dl; mov rcx, *; add dl, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Find("mov dl, 0x1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (StaticGetItemRow)
        return StaticGetItemRow(WeaponDefinition, Direction);

    static auto GetItemRow = Anchor::Function<FWeaponUpgradeItemRow* (*)(const ABuildingItemWeaponUpgradeActor*, UFortWeaponItemDefinition*, EFortWeaponUpgradeDirection)>(TEXT("ABuildingItemWeaponUpgradeActor::GetItemRow"))
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("setz r8b; inc r8b; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetItemRow(this, WeaponDefinition, Direction);
}

int32 ABuildingItemWeaponUpgradeActor::GetCostForRessource(const uint8* UpgradeCost) const
{
    DETOUR(ABuildingItemWeaponUpgradeActor, GetCostForRessource, Anchor::Hook()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov rcx, *; mov *, eax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(ABuildingItemWeaponUpgradeActor::StaticClass() && !AreConsumedItemsRemovedByGame())
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Caller()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Inline());

    static auto BrickCost = FWeaponUpgradeItemRow::StaticStruct()->FindPropertyByName(FName(TEXT("BrickCost")));

    UpgradeItemRowBeingConsumed = (FWeaponUpgradeItemRow*)(UpgradeCost - BrickCost->GetOffset_ForInternal());

    return Original(UpgradeCost);
}

bool ABuildingItemWeaponUpgradeActor::AreConsumedItemsRemovedByGame()
{
    static auto bRemovesItems = !::Anchor::Exists()
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov rcx, *; mov *, eax; call 0x*")
        .Find("call *", 2)
        .Is("call [*]")
        .Or()
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call *", 2)
        .Is("call [*]")
        .Or()
        .String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory."))
        .Start()
        .Caller()
        .Find("lea rdx, [*+0x1a]; mov *, eax; mov rcx, *; call 0x*")
        .Find("call *", 2)
        .Is("call [*]")
        .Get();

    return bRemovesItems;
}

UFortResourceItemDefinition* ABuildingItemWeaponUpgradeActor::GetWoodItem() const
{
    static auto WoodItem = GetClass()->FindPropertyByName(FName(TEXT("WoodItem")));

    return *WoodItem->ContainerPtrToValuePtr<UFortResourceItemDefinition*>(this);
}

UFortResourceItemDefinition* ABuildingItemWeaponUpgradeActor::GetMetalItem() const
{
    static auto MetalItem = GetClass()->FindPropertyByName(FName(TEXT("MetalItem")));

    return *MetalItem->ContainerPtrToValuePtr<UFortResourceItemDefinition*>(this);
}

UFortResourceItemDefinition* ABuildingItemWeaponUpgradeActor::GetBrickItem() const
{
    static auto BrickItem = GetClass()->FindPropertyByName(FName(TEXT("BrickItem")));

    return *BrickItem->ContainerPtrToValuePtr<UFortResourceItemDefinition*>(this);
}

UScriptStruct* FWeaponUpgradeItemRow::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.WeaponUpgradeItemRow"));

    return Struct;
}

UFortWeaponItemDefinition* FWeaponUpgradeItemRow::GetCurrentWeaponDef() const
{
    static auto CurrentWeaponDef = StaticStruct()->FindPropertyByName(FName(TEXT("CurrentWeaponDef")));

    return *CurrentWeaponDef->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}

const uint8* FWeaponUpgradeItemRow::GetWoodCost() const
{
    static auto WoodCost = StaticStruct()->FindPropertyByName(FName(TEXT("WoodCost")));

    return WoodCost->ContainerPtrToValuePtr<uint8>(this);
}

const uint8* FWeaponUpgradeItemRow::GetMetalCost() const
{
    static auto MetalCost = StaticStruct()->FindPropertyByName(FName(TEXT("MetalCost")));

    return MetalCost->ContainerPtrToValuePtr<uint8>(this);
}

const uint8* FWeaponUpgradeItemRow::GetBrickCost() const
{
    static auto BrickCost = StaticStruct()->FindPropertyByName(FName(TEXT("BrickCost")));

    return BrickCost->ContainerPtrToValuePtr<uint8>(this);
}

UFortWeaponItemDefinition* FWeaponUpgradeItemRow::GetUpgradedWeaponDef() const
{
    static auto UpgradedWeaponDef = StaticStruct()->FindPropertyByName(FName(TEXT("UpgradedWeaponDef")));

    return *UpgradedWeaponDef->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}
