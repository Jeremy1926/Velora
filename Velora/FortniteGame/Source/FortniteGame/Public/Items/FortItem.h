#pragma once

#include "UObject/Object.h"
#include "Misc/Guid.h"

class UFortItemDefinition;
enum class EFortItemType : uint8;

class UFortItem : public UObject
{
public:
    UFortItemDefinition* GetItemDefinitionBP() const;

    EFortItemType GetType() const;

    FGuid GetItemGuid() const;

    FGuid GetTrackerGuid() const;

    bool CanBeDropped() const;

    int32 GetNumInStack() const;
};
