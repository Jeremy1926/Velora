#pragma once

#include "UObject/WeakObjectPtr.h"

class UBodySetup;
class UBodySetupCore;

struct FBodyInstanceCore
{
    virtual ~FBodyInstanceCore() = default;

    TWeakObjectPtr<UBodySetupCore> BodySetup;
};

struct FBodyInstance : public FBodyInstanceCore
{
    UBodySetup* GetBodySetup() const
    {
        return (UBodySetup*)BodySetup.Get();
    }
};
