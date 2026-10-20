#pragma once

#include "AIController.h"
#include "Containers/Array.h"

class AFortInventory;
class UFortAthenaAIBotInventoryItems;
struct FFortAthenaLoadout;
struct FItemAndCount;

class AFortAthenaAIBotController : public AAIController
{
public:
    static UClass* StaticClass();

    bool CreateInventory();

    void GiveInventory(const TArray<FItemAndCount>& Items);

    void InitializeInventory(UFortAthenaAIBotInventoryItems* InStartupInventory);

    AFortInventory*& GetInventory();

    UFortAthenaAIBotInventoryItems* GetStartupInventory() const;

    FFortAthenaLoadout& GetCosmeticLoadoutBC();
};
