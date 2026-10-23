#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortQuestItem;
class UFortQuestItemDefinition;

class UFortQuestManager : public UObject
{
public:
    static UClass* StaticClass();

    bool HasCompletedTransientQuest(const UFortQuestItemDefinition* Definition, bool bMineOnly) const;

    const TArray<UFortQuestItem*>& GetActiveTransientQuests() const;
};
