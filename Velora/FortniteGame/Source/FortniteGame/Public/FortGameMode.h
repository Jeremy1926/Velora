#pragma once

#include "GameFramework/GameMode.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"

class AFortGameMode : public AGameMode
{
public:
    void FinishWorldInitialization();

    void OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions);
};
