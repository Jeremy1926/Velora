#pragma once

#include "AISubsystem.h"

class UAthenaAISettings;

class UAthenaAIPerceptionManager : public UAISubsystem
{
public:
    void OnAISettingsUpdated(UAthenaAISettings* AISettings);
};
