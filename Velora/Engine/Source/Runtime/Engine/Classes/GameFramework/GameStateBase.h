#pragma once

#include "GameFramework/Info.h"
#include "Containers/Array.h"

class APlayerState;

class AGameStateBase : public AInfo
{
public:
    TArray<APlayerState*>& GetPlayerArray();

    float GetServerWorldTimeSeconds() const;
};
