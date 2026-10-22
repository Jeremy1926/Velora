#pragma once

#include "UObject/Object.h"

class AController;

class UFortControllerRequirement : public UObject
{
public:
    static UClass* StaticClass();

    bool IsRequirementMet(const AController* Controller) const;
};
