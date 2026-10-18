#pragma once

#include "UObject/Object.h"

class APlayerController;

class UPlayer : public UObject
{
public:
    APlayerController* GetPlayerController() const;

    int32 GetCurrentNetSpeed() const;
};
