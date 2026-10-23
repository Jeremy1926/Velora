#pragma once

#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"

class ASimObject : public AActor
{
public:
    static TMulticastDelegate<void, AActor*>& GetNotifyNetCullDistanceChange();
};
