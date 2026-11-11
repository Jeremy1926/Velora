#pragma once

#include "UObject/Object.h"

class AActor;
class AFortPlayerController;

class UFortIrisHelper : public UObject
{
public:
    static UClass* StaticClass();

    static UFortIrisHelper* Get();

    void OnPlayerControllerInventorySwitched(AFortPlayerController* PC);

    void OnPlayerControllerViewTargetChanged(AFortPlayerController* PC, AActor* NewViewTarget, AActor* OldViewTarget);
};
