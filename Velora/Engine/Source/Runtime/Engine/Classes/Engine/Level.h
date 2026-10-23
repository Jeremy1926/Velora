#pragma once

#include "UObject/Object.h"

class AActor;

class ULevel : public UObject
{
public:
    UWorld* GetOwningWorld() const;

    bool IsAssociatingLevel() const;

    bool HasVisibilityChangeRequestPending() const;

    static bool IsNetActor(const AActor* Actor);
};
