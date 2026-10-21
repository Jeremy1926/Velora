#pragma once

#include "Containers/Array.h"

class UClass;
struct FItemAndCount;

class IFortInventoryOwnerInterface
{
public:
    static UClass* StaticClass();

    bool CreateInventory();

    void InitializeInventory(const TArray<FItemAndCount>& Items);
};
