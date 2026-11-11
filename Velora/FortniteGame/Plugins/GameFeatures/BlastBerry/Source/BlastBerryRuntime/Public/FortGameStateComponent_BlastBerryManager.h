#pragma once

#include "Components/ActorComponent.h"

class UObject;

class UFortGameStateComponent_BlastBerryManager : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_BlastBerryManager* Get(const UObject* WorldContextObject);

    void HandleTeamRemovedFromGame(uint8 TeamId);

    float GetServerTimeForRespawnsDisabled() const;
};
