#pragma once

#include "UObject/Object.h"
#include "Misc/Guid.h"

class UFortItemDefinition;

class UFortItem : public UObject
{
public:
    UFortItemDefinition* GetItemDefinitionBP() const;

    FGuid GetItemGuid() const;
};
