#pragma once

#include "Containers/Array.h"

class AFortInventory;
class UClass;
struct FItemAndCount;

class IFortInventoryOwnerInterface
{
public:
    static UClass* StaticClass();

    AFortInventory* GetWorldInventory();

    bool CreateInventory();

    void InitializeInventory(const TArray<FItemAndCount>& Items);
};
