#pragma once

#include "HAL/Platform.h"

class UObject;

class FScriptInterface
{
public:
    FORCEINLINE UObject* GetObject() const
    {
        return ObjectPointer;
    }

private:
    UObject* ObjectPointer;
    void* InterfacePointer;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
};
