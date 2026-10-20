#pragma once

#include "AIController.h"
#include "Containers/Array.h"

class AFortInventory;
class UBehaviorTree;
class UFortAthenaAIBotInventoryItems;
class UFortBotNameSettings;
class UFortItemDefinition;
class UFortWorldItem;
class UScriptStruct;
struct FFortAthenaLoadout;
struct FItemAndCount;
enum class EFortItemType : uint8;
enum class EFortWeaponType : uint8;

struct FFortBotInventoryInfo
{
    static UScriptStruct* StaticStruct();

    UFortItemDefinition*& GetItemDefinition();

    UFortWorldItem*& GetFortItem();

    EFortItemType& GetItemType();

    EFortWeaponType& GetWeaponType();
};

class AFortAthenaAIBotController : public AAIController
{
public:
    static UClass* StaticClass();

    bool CreateInventory();

    void GiveInventory(const TArray<FItemAndCount>& Items);

    void InitializeInventory(UFortAthenaAIBotInventoryItems* InStartupInventory);

    bool WouldItemFitInBotInventory(UFortWorldItem* Item, int32& OutSlotIndex);

    void ApplyCharacterCustomization(const FFortAthenaLoadout* InCustomizationLoadout, const UFortBotNameSettings* InBotNameSettings, bool bRequiresUniqueNetId);

    FFortBotInventoryInfo& GetSlotItems(int32 Index);

    bool GetbAllowUnsupportedItemsInDefaultInventory() const;

    AFortInventory*& GetInventory();

    UFortAthenaAIBotInventoryItems* GetStartupInventory() const;

    UBehaviorTree* GetBehaviorTree() const;

    FFortAthenaLoadout& GetCosmeticLoadoutBC();
};
