#pragma once

#include "UObject/Class.h"

template<class TClass>
class TSubclassOf
{
public:
    FORCEINLINE TSubclassOf()
        : Class(nullptr)
    {
    }

    FORCEINLINE TSubclassOf(UClass* From)
        : Class(From)
    {
    }

    FORCEINLINE TClass* GetDefaultObject() const
    {
        UObject* Result = nullptr;

        if (Class)
        {
            Result = Class->GetDefaultObject();
        }

        return (TClass*)Result;
    }

    FORCEINLINE operator UClass*() const
    {
        return Class;
    }

private:
    UClass* Class;
};
