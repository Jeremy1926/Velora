#pragma once

#include "Player/FortPlayerControllerPvP.h"

class AFortPlayerState;
class UAthenaMarkerComponent;
class UAthenaPickaxeItemDefinition;

struct FFortAthenaLoadout
{
    UAthenaPickaxeItemDefinition*& GetPickaxe();
};

class AFortPlayerControllerAthena : public AFortPlayerControllerPvP
{
public:
    typedef AFortPlayerControllerPvP Super;

    static UClass* StaticClass();

    void ServerRestartPlayer_Implementation();

    FFortAthenaLoadout* GetCustomizationLoadout();

    void ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution);

    UAthenaMarkerComponent* GetMarkerComponent() const;

    uint8 GetSquadId() const;

    AFortPlayerState* GetSkydiveLeader() const;
};
