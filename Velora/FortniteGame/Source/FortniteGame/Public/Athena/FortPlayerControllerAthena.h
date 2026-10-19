#pragma once

#include "Player/FortPlayerControllerPvP.h"

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
};
