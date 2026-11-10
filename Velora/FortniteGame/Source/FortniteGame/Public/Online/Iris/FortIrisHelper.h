#pragma once

#include "UObject/Object.h"

class AFortPlayerController;

class UFortIrisHelper : public UObject
{
public:
    static UClass* StaticClass();

    static UFortIrisHelper* Get();

    void OnPlayerControllerInventorySwitched(AFortPlayerController* PC);
};
