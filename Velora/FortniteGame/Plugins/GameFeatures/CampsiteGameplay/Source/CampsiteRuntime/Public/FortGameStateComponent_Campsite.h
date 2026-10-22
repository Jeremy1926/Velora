#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"

struct FFrame;
class UObject;
class AAbandonedCampsitePlacedSpawner;

class UFortGameStateComponent_Campsite : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_Campsite* Get(const UObject* WorldContextObject);

    void execClaimUnusedBotName(FFrame& Stack, void* const Z_Param__Result);

    void ClaimUnusedBotName(FString& OutBotName);

    TArray<AAbandonedCampsitePlacedSpawner*>& GetPlacedCampsiteSpawnPoints();
};
