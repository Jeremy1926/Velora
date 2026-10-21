#pragma once

#include "Components/ActorComponent.h"

class UFortAthenaLivingWorldManager : public UActorComponent
{
public:
    static UClass* StaticClass();

    static int32 GetEventInstancesOffset();
};
