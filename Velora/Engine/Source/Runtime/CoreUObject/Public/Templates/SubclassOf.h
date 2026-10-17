#pragma once

#include "UObject/Class.h"

template<class TClass>
class TSubclassOf
{
public:
    FORCEINLINE TClass* GetDefaultObject() const
    {
        UObject* Result = nullptr;

        if (Class)
        {
            Result = Class->GetDefaultObject();
        }

        return (TClass*)Result;
    }

private:
    UClass* Class;
};
