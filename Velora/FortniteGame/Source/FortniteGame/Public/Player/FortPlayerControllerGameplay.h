#pragma once

#include "Player/FortPlayerController.h"

class UFortWorldItemDefinition;
class UScriptStruct;

struct FGhostModeRepData
{
    static UScriptStruct* StaticStruct();

    bool GetInGhostMode() const;

    UFortWorldItemDefinition* GetGhostModeItemDef() const;
};

class AFortPlayerControllerGameplay : public AFortPlayerController
{
public:
    FGhostModeRepData& GetGhostModeRepData();

    void StartGhostMode(UFortWorldItemDefinition* ItemProvidingGhostMode);
};
