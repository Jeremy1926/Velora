#pragma once

#include "Components/ActorComponent.h"

class UFortControllerComponent_Interaction : public UActorComponent
{
public:
    static UClass* StaticClass();

    float GetInteractDistance() const;
};
