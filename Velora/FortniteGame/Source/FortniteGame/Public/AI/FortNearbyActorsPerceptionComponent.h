#pragma once

#include "Components/ActorComponent.h"
#include "Engine/EngineBaseTypes.h"

class UFortNearbyActorsPerceptionComponent : public UActorComponent
{
public:
    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
};
