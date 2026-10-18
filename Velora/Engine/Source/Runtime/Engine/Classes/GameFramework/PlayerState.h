#pragma once

#include "GameFramework/Info.h"
#include "Containers/UnrealString.h"

class APlayerState : public AInfo
{
public:
    FString GetPlayerName() const;
};
