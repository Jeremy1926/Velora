#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"

class UFortAthenaAISpawnerDataComponentList;

class UFortAthenaAISpawnerData : public UObject
{
public:
    static UClass* StaticClass();

    static UFortAthenaAISpawnerDataComponentList* CreateComponentListFromClass(UClass* AISpawnerDataClass, UObject* OuterObject);

    FGameplayTagContainer& GetDescriptorTag();
};
