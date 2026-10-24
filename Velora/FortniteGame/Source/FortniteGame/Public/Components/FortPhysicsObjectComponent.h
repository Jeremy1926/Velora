#pragma once

#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"

class UFortPhysicsObjectComponent : public UActorComponent
{
public:
    static TMulticastDelegate<void, UFortPhysicsObjectComponent*>& GetNotifyPhysicsObjectComponentActivated();

    static TMulticastDelegate<void, UFortPhysicsObjectComponent*>& GetNotifyPhysicsObjectComponentDeactivated();

    float GetInactivateStateNetUpdateFrequency() const;
};
