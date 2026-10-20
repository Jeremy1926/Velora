#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortItemDefinition;

class UFortAthenaAIBotInventoryItems : public UObject
{
public:
    TArray<UFortItemDefinition*>& GetItems();
};
