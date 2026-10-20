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
        .Reference("mov rdx, *")
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
        .Reference("mov rdx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"))
        .Call()
        .FieldFind("cmp *[*+%x], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded"))))
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor")));

    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto WeaponUpgradeMachineSpawnChance = MapInfo ? MapInfo->GetWeaponUpgradeMachineSpawnChance() : nullptr;

    if (!HasAuthority() || !WeaponUpgradeMachineSpawnChance)
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
        .LastCall(Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer")).Find("jnz 0x*").Target())
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingItemCollectorActor::GrantOutput"))
        .InVTable(TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor")));

    auto PlayerController = Cast<AFortPlayerControllerAthena>(Player);
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!HasAuthority() || !WorldInventory || !Item)
        return;

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
