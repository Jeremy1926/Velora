#pragma once

#include "Components/ActorComponent.h"

class UFortAthenaAIRuntimeParametersComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    UObject* GetOrCreateAIRuntimeParameters(UClass* RuntimeParametersClass, int32 SkillLevel, uint8 ConflictBehavior);
};
