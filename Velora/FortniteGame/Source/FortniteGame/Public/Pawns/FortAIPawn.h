#pragma once

#include "Pawns/FortPawn.h"

class AFortInventory;
struct FFrame;

class AFortAIPawn : public AFortPawn
{
public:
    static UClass* StaticClass();

    void HandleDeathNotifications();

    void SetLootDropData(float LootDropChance, FName WorldItemTierGroup);

    void DropLoot();

    void GiveAmmo(const int32 AmmoQuantity);

    void execGiveAmmo(FFrame& Stack, void* const Z_Param__Result);

    void DropAllItems(bool bSpawnPickups);

    void execDropAllItems(FFrame& Stack, void* const Z_Param__Result);

    AFortInventory* GetWorldInventory() const;

    bool GetHasDroppedLoot() const;

    void SetHasDroppedLoot(bool bInHasDroppedLoot);
};
