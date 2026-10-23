#pragma once

#include "GameFramework/Actor.h"

class AFortAthenaPointAroundPlayerProvider : public AActor
{
public:
    static UClass* StaticClass();

    void EnablePointProvider();
};
