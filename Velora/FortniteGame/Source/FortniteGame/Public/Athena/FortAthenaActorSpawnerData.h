#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"

class UFortAthenaActorSpawnerData : public UObject
{
public:
    static UClass* StaticClass();

    UClass* GetActorClass();

    FGameplayTagContainer& GetDescriptorTag();
};
