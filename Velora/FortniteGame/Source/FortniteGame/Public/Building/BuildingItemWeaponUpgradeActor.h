#pragma once

#include "Building/BuildingItemCollectorActor.h"

class AFortPlayerController;
class UFortWeaponItemDefinition;
class UFortWorldItemDefinition;
class UScriptStruct;

enum class EFortWeaponUpgradeDirection : uint8
{
    NotSet,
    Vertical,
    Horizontal,
};

struct FWeaponUpgradeItemRow
{
    static UScriptStruct* StaticStruct();

    UFortWeaponItemDefinition* GetUpgradedWeaponDef() const;
};

class ABuildingItemWeaponUpgradeActor : public ABuildingItemCollectorActor
{
public:
    static UClass* StaticClass();

    void AthenaPostApplyPlaylistDataTables();

    void GrantOutput(AFortPlayerController* Player, UFortWorldItemDefinition* Item);

    FWeaponUpgradeItemRow* GetItemRow(UFortWeaponItemDefinition* WeaponDefinition, EFortWeaponUpgradeDirection Direction) const;
};
