#pragma once

#include "UObject/UObjectBaseUtility.h"

class UFunction;
class UWorld;
struct FOutParmRec;
struct FFrame;

class UObject : public UObjectBaseUtility
{
public:
    static UClass* StaticClass();

    UWorld* GetWorld() const;

    bool NeedsLoadForServer() const;

    static bool CanCreateInCurrentContext(UObject* Template);

    static void PatchCanCreateInCurrentContext();

    FORCEINLINE UObject* GetArchetype() const
    {
        return GetArchetypeFromRequiredInfo(GetClass(), GetOuter(), GetFName(), GetFlags());
    }

    static UObject* GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags);

    bool CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack);
};
