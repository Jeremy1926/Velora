#pragma once

#include "AIController.h"
#include "Containers/Array.h"

class AFortInventory;
class UBehaviorTree;
class UFortAthenaAIBotInventoryItems;
class UFortBotNameSettings;
struct FFortAthenaLoadout;
struct FItemAndCount;

class AFortAthenaAIBotController : public AAIController
{
public:
    static UClass* StaticClass();

    bool CreateInventory();

    void GiveInventory(const TArray<FItemAndCount>& Items);

    void InitializeInventory(UFortAthenaAIBotInventoryItems* InStartupInventory);

    void ApplyCharacterCustomization(const FFortAthenaLoadout* InCustomizationLoadout, const UFortBotNameSettings* InBotNameSettings, bool bRequiresUniqueNetId);

    AFortInventory*& GetInventory();

    UFortAthenaAIBotInventoryItems* GetStartupInventory() const;

    UBehaviorTree* GetBehaviorTree() const;

    FFortAthenaLoadout& GetCosmeticLoadoutBC();
};
