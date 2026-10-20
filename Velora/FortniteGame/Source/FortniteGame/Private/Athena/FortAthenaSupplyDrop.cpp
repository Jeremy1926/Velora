#include "pch.h"
#include "Athena/FortAthenaSupplyDrop.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto SpawnPickup = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup")) ? DetourCall(TEXT("AFortAthenaSupplyDrop::SpawnPickup"), Scanner::Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup")).Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*").Find("call 0x*").Get(), &AFortAthenaSupplyDrop::SpawnPickup) : nullptr;

AFortPickup* AFortAthenaSupplyDrop::SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const FVector& Position, const FVector& Direction)
{
    if (!ItemDefinition || NumberToSpawn <= 0)
        return nullptr;

    FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, 0);

    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

    if (ItemDefinition->GetItemType() == WeaponMelee || ItemDefinition->GetItemType() == WeaponRanged)
    {
        ItemEntry.SetDurability(ItemDefinition->GetMaxDurability(ItemEntry.GetLevel()));
        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
    }

    auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), this, EFortPickupSourceTypeFlag::Container);

    if (Pickup)
        Pickup->TossPickup(Position, nullptr, 0, true, true, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset);

    return Pickup;
}
