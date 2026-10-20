#pragma once

#include "Building/BuildingItemCollectorActor.h"

class AFortPlayerController;
class UFortWorldItemDefinition;

class ABuildingItemWeaponUpgradeActor : public ABuildingItemCollectorActor
{
public:
    static UClass* StaticClass();

    void AthenaPostApplyPlaylistDataTables();

    void GrantOutput(AFortPlayerController* Player, UFortWorldItemDefinition* Item);
};
