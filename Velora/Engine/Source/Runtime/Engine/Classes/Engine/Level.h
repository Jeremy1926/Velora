#pragma once

#include "UObject/Object.h"

class AActor;

class ULevel : public UObject
{
public:
    static UClass* StaticClass();

    UWorld* GetOwningWorld() const;

    bool IsPersistentLevel() const;

    bool IsAssociatingLevel() const;

    bool HasVisibilityChangeRequestPending() const;

    static bool IsNetActor(const AActor* Actor);
};
