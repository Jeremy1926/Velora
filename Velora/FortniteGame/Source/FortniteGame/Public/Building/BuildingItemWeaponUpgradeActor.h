#pragma once

#include "Building/BuildingItemCollectorActor.h"

class AFortPlayerController;
class UFortResourceItemDefinition;
class UFortWeaponItemDefinition;
class UFortWorldItemDefinition;
class UScriptStruct;

enum class EFortWeaponUpgradeDirection : uint8
{
    NotSet,
    Vertical,
    Horizontal,
};

enum class EFortWeaponUpgradeInteractionResult : uint8
{
    Upgradable,
    NotEnoughResources,
    CannotUpgrade,
    CannotInteract,
};

struct FWeaponUpgradeItemRow
{
    static UScriptStruct* StaticStruct();

    UFortWeaponItemDefinition* GetCurrentWeaponDef() const;

    UFortWeaponItemDefinition* GetUpgradedWeaponDef() const;

    const uint8* GetWoodCost() const;

    const uint8* GetMetalCost() const;

    const uint8* GetBrickCost() const;
};

class ABuildingItemWeaponUpgradeActor : public ABuildingItemCollectorActor
{
public:
    static UClass* StaticClass();

    void AthenaPostApplyPlaylistDataTables();

    void GrantOutput(AFortPlayerController* Player, UFortWorldItemDefinition* Item);

    FWeaponUpgradeItemRow* GetItemRow(UFortWeaponItemDefinition* WeaponDefinition, EFortWeaponUpgradeDirection Direction) const;

    int32 GetCostForRessource(const uint8* UpgradeCost) const;

    static bool AreConsumedItemsRemovedByGame();

    UFortResourceItemDefinition* GetWoodItem() const;

    UFortResourceItemDefinition* GetMetalItem() const;

    UFortResourceItemDefinition* GetBrickItem() const;
};
