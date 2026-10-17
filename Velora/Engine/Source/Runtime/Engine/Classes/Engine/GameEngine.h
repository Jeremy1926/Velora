#pragma once

#include "Engine/Engine.h"

class UGameInstance;

class UGameEngine : public UEngine
{
public:
    UGameInstance* GetGameInstance() const;

    void Tick(float DeltaSeconds, bool bIdleMode);
};
