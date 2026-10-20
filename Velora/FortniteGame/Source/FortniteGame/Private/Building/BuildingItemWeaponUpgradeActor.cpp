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

static auto ConsumeItems = ABuildingItemWeaponUpgradeActor::StaticClass() ? []
{
    auto ConsumeItems = Scanner::String(TEXT("ABuildingItemWeaponUpgradeActor::ConsumeItems - The Player's current weapon is no longer in the inventory.")).Start();
    auto K2_RemoveItemFromPlayer = Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:K2_RemoveItemFromPlayer")).Find("jnz 0x*").Target(TEXT("UFortKismetLibrary::K2_RemoveItemFromPlayer"));
    Scanner::FCode LastRemoveItem;

    for (auto Call = ConsumeItems.Find("call 0x*"); K2_RemoveItemFromPlayer && Call.Get(); Call = Call.Find("call 0x*"))
    {
        if (Call.Target() != K2_RemoveItemFromPlayer)
            continue;

        DetourCall(TEXT("UFortKismetLibrary::K2_RemoveItemFromPlayer"), Call.Get(), &UFortKismetLibrary::K2_RemoveItemFromPlayer);
        LastRemoveItem = Call;
    }

    return LastRemoveItem;
}() : Scanner::FCode();

static auto GrantOutput = ABuildingItemWeaponUpgradeActor::StaticClass() ? []
{
    auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"));

    return DetourVTable(TEXT("ABuildingItemWeaponUpgradeActor::GrantOutput"), VTable, ::ConsumeItems.Find("call [*+0x*]").Slot(TEXT("ABuildingItemCollectorActor::GrantOutput")), &ABuildingItemWeaponUpgradeActor::GrantOutput);
}() : nullptr;

static auto AthenaPostApplyPlaylistDataTables = ABuildingItemWeaponUpgradeActor::StaticClass() ? []
{
    auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target(TEXT("ENGINE_ReceiveBeginPlay"));
    auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get(TEXT("AActor::ReceiveBeginPlay"));
    auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get(TEXT("AActor::BeginPlay"));
    auto ActorVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
    auto BeginPlay = Scanner::Slot(TEXT("AActor::BeginPlay"), ActorVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == ActorBeginPlay; });
    auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingItemWeaponUpgradeActor"));
    auto PlaylistDataIsLoaded = std::format("cmp byte ptr [*+{:#x}], *", AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("bPlaylistDataIsLoaded")))->GetOffset_ForInternal());
    auto CollectorBeginPlay = Scanner::At(Scanner::IsSlot(BeginPlay) ? VTable[BeginPlay / sizeof(void*)] : nullptr).Call();

    return DetourVTable(TEXT("ABuildingItemWeaponUpgradeActor::AthenaPostApplyPlaylistDataTables"), VTable, CollectorBeginPlay.Find(PlaylistDataIsLoaded).Find("call [rax+0x*]").Slot(TEXT("ABuildingItemCollectorActor::AthenaPostApplyPlaylistDataTables")), &ABuildingItemWeaponUpgradeActor::AthenaPostApplyPlaylistDataTables);
}() : nullptr;

UClass* ABuildingItemWeaponUpgradeActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingItemWeaponUpgradeActor"));

    return Class;
}

void ABuildingItemWeaponUpgradeActor::AthenaPostApplyPlaylistDataTables()
{
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
