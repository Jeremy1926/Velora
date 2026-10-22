#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"
#include "Containers/Set.h"
#include "UObject/SoftObjectPtr.h"

struct FFrame;
class UObject;
class AAbandonedCampsitePlacedSpawner;
class UFortItemDefinition;

class UFortGameStateComponent_Campsite : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_Campsite* Get(const UObject* WorldContextObject);

    void BeginPlay();

    void CacheAllowedItemDefinitions();

    void execClaimUnusedBotName(FFrame& Stack, void* const Z_Param__Result);

    void ClaimUnusedBotName(FString& OutBotName);

    TArray<AAbandonedCampsitePlacedSpawner*>& GetPlacedCampsiteSpawnPoints();

    TSet<UFortItemDefinition*>& GetAllowedItemDefinitions();

    TArray<TSoftObjectPtr<UFortItemDefinition>>& GetBlacklistedItemDefinitions();

    TArray<FString>& GetBlacklistedItemPaths();
};
