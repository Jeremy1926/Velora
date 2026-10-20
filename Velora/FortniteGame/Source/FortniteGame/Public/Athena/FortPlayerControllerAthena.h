#pragma once

#include "Player/FortPlayerControllerPvP.h"

class AFortPlayerState;
class UAthenaBackpackItemDefinition;
class UAthenaCharacterItemDefinition;
class UAthenaGliderItemDefinition;
class UAthenaMarkerComponent;
class UAthenaPickaxeItemDefinition;
class UAthenaSkyDiveContrailItemDefinition;

struct FFortAthenaLoadout
{
    static UScriptStruct* StaticStruct();

    UAthenaCharacterItemDefinition*& GetCharacter();

    UAthenaGliderItemDefinition*& GetGlider();

    UAthenaSkyDiveContrailItemDefinition*& GetSkyDiveContrail();

    UAthenaBackpackItemDefinition*& GetBackpack();

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

    void ClientReportPhaseFound(const FString& Phase);

    UAthenaMarkerComponent* GetMarkerComponent() const;

    uint8 GetSquadId() const;

    AFortPlayerState* GetSkydiveLeader() const;

    void RespawnPlayerAfterDeath(bool bEnterSkydiving);
};
