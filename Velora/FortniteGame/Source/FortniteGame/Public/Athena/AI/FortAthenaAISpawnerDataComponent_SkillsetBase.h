#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortAthenaAISpawnerDataComponent_SkillsetBase : public UObject
{
public:
    float GetSkill();

    TArray<UClass*> GetSkillSets();

    bool ShouldUseMatchMMRToOverrideSkillLevel();
};
