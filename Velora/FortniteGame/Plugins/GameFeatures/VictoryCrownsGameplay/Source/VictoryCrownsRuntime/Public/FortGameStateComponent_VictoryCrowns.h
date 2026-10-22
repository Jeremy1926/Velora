#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"

class UObject;
class AFortPlayerStateAthena;

class UFortGameStateComponent_VictoryCrowns : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_VictoryCrowns* Get(const UObject* WorldContextObject);

    void AddPlayerState(AFortPlayerStateAthena* PlayerStateToAdd);

    TArray<AFortPlayerStateAthena*>& GetCrownBearerPlayerStates();
};
