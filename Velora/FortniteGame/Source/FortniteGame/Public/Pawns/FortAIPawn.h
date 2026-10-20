#pragma once

#include "Pawns/FortPawn.h"

class AFortAIPawn : public AFortPawn
{
public:
    static UClass* StaticClass();

    void HandleDeathNotifications();

    void SetLootDropData(float LootDropChance, FName WorldItemTierGroup);

    void DropLoot();

    bool GetHasDroppedLoot() const;

    void SetHasDroppedLoot(bool bInHasDroppedLoot);
};
