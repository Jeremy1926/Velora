#pragma once

#include "Player/FortPlayerControllerPvP.h"

class UAthenaMarkerComponent;
class UAthenaPickaxeItemDefinition;

struct FFortAthenaLoadout
{
    UAthenaPickaxeItemDefinition*& GetPickaxe();
};

class AFortPlayerControllerAthena : public AFortPlayerControllerPvP
{
public:
    static UClass* StaticClass();

    FFortAthenaLoadout* GetCustomizationLoadout();

    void ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution);

    UAthenaMarkerComponent* GetMarkerComponent() const;

    uint8 GetSquadId() const;
};
