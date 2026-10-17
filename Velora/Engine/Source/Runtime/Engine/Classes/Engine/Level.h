#pragma once

#include "UObject/Object.h"

class ULevel : public UObject
{
public:
    UWorld* GetOwningWorld() const;

    bool IsAssociatingLevel() const;

    bool HasVisibilityChangeRequestPending() const;
};
