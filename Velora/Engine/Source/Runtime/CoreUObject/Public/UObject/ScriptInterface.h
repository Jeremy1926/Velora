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

    FORCEINLINE void SetObject(UObject* InObjectPointer)
    {
        ObjectPointer = InObjectPointer;

        if (!ObjectPointer)
            SetInterface(nullptr);
    }

    FORCEINLINE void SetInterface(void* InInterfacePointer)
    {
        InterfacePointer = InInterfacePointer;
    }

private:
    UObject* ObjectPointer;
    void* InterfacePointer;
};

template<class InterfaceType>
class TScriptInterface : public FScriptInterface
{
};
