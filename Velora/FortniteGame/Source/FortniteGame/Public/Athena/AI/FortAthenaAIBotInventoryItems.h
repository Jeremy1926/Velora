#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class UFortItemDefinition;
struct FItemAndCount;

class UFortAthenaAIBotInventoryItems : public UObject
{
public:
    static UClass* StaticClass();

    static TArray<FItemAndCount> GetItems(const TArray<FItemAndCount>& InItems);

    TArray<FItemAndCount> GetItems();
};
