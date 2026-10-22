#pragma once

#include "Items/FortItemDefinition.h"
#include "Containers/Array.h"
#include "UObject/SoftObjectPtr.h"

class UFortQuestItemDefinition : public UFortItemDefinition
{
public:
    static UClass* StaticClass();

    const TArray<TSoftObjectPtr<UFortQuestItemDefinition>>& GetTransientPrerequisiteQuests() const;
};
