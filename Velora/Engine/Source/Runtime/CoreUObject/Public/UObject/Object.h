#pragma once

#include "UObject/UObjectBaseUtility.h"

class UWorld;

class UObject : public UObjectBaseUtility
{
public:
    UWorld* GetWorld() const;

    FORCEINLINE UObject* GetArchetype() const
    {
        return GetArchetypeFromRequiredInfo(GetClass(), GetOuter(), GetFName(), GetFlags());
    }

    static UObject* GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags);
};
