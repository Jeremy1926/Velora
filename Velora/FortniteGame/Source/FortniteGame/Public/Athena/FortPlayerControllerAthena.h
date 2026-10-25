#pragma once

#include "Player/FortPlayerControllerPvP.h"

class AController;
class AFortBroadcastRemoteClientInfo;
class AFortPlayerState;
class UAthenaBackpackItemDefinition;
class UAthenaCharacterItemDefinition;
class UAthenaGliderItemDefinition;
class UAthenaMarkerComponent;
class UAthenaPickaxeItemDefinition;
class UAthenaSkyDiveContrailItemDefinition;
class UFortWorldItem;
class UFortWorldItemDefinition;

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

    void ServerAttemptAircraftJump(const FRotator& ClientRotation);

    void OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher);

    bool GetMarkedAlive() const;

    void SetMatchPlacement(int32 Placement);

    FFortAthenaLoadout* GetCustomizationLoadout();

    void ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution);

    void ClientReportPhaseFound(const FString& Phase);

    UAthenaMarkerComponent* GetMarkerComponent() const;

    uint8 GetSquadId() const;

    AFortPlayerState* GetSkydiveLeader() const;

    AFortBroadcastRemoteClientInfo* GetBroadcastRemoteClientInfo() const;

    void RespawnPlayerAfterDeath(bool bEnterSkydiving);

    static bool ExternalShouldAlwaysDropItemOnDeathOrLogout(const UFortWorldItem* Item, bool bIgnoreItemMutators);

    static void ExternalShouldDropOrDestroyByItemType(AController* Controller, const UFortWorldItemDefinition* ItemDefinition, bool* bShouldDrop, bool* bShouldDestroy);
};
