#pragma once

#include "GameFramework/Actor.h"

class AFortPlayerControllerZone;

class AFortPoiManager : public AActor
{
public:
    static UClass* StaticClass();

    void CheckPlayerControllerWithPoiGrid(AFortPlayerControllerZone* PlayerController);
};
