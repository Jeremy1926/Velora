#pragma once

#include "UObject/UObjectBaseUtility.h"

class UFunction;
class UWorld;

class UObject : public UObjectBaseUtility
{
public:
    static UClass* StaticClass();

    UWorld* GetWorld() const;

    FORCEINLINE UObject* GetArchetype() const
    {
        return GetArchetypeFromRequiredInfo(GetClass(), GetOuter(), GetFName(), GetFlags());
    }

    static UObject* GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags);

    UFunction* FindFunctionChecked(FName InName) const;

    void ProcessEvent(UFunction* Function, void* Parms);
};
