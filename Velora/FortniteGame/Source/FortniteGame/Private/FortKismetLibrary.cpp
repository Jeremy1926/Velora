#include "pch.h"
#include "FortKismetLibrary.h"
#include "Player/FortPlayerController.h"
#include "Items/FortInventory.h"
#include "Items/FortQuickBars.h"
#include "Items/FortWorldItemDefinition.h"
#include "Scanner.h"

void UFortKismetLibrary::K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive)
{
    if (!PlayerController || !ItemDefinition)
        return;

    auto WorldInventory = PlayerController->GetWorldInventory();

    if (!WorldInventory)
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.ReplicatedEntries) FFortItemEntry(ItemDefinition, NumberToGive, ItemDefinition->GetMinLevel());

    Inventory.MarkItemDirty(*ItemEntry);
    WorldInventory->HandleInventoryLocalUpdate();

    PlayerController->AddItemToQuickBars(ItemDefinition, EFortQuickBars::Max_None, -1);
}

void UFortKismetLibrary::BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier)
{
    static auto BroadcastSoundAtLocation = (void (*)(UObject*, USoundBase*, const FVector&, float, float))Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:BroadcastSoundAtLocation")).Get();

    BroadcastSoundAtLocation(WorldContextObject, InSound, Location, VolumeMultiplier, PitchMultiplier);
}
