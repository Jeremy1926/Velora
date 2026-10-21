#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortItemDefinition;
struct FItemAndCount;

class UFortAthenaAIBotInventoryItems : public UObject
{
public:
    TArray<FItemAndCount> GetItems();
};
