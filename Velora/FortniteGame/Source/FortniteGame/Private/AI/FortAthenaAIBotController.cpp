#include "pch.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/AI/FortAthenaAIBotInventoryItems.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaAIBotController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIBotController"));

    return Class;
}

bool AFortAthenaAIBotController::CreateInventory()
{
    if (GetInventory())
        return false;

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = this;

    GetInventory() = (AFortInventory*)GetWorld()->SpawnActor(AFortInventory::StaticClass(), nullptr, nullptr, SpawnParameters);

    if (!GetInventory())
        return false;

    GetInventory()->GetInventoryType() = EFortInventoryType::World;

    return true;
}

void AFortAthenaAIBotController::GiveInventory(const TArray<FItemAndCount>& Items)
{
    auto Inventory = GetInventory();

    if (!Inventory)
        return;

    for (auto& ItemAndCount : Items)
    {
        if (!ItemAndCount.Item || ItemAndCount.Count <= 0)
            continue;

        auto ItemEntry = new(Inventory->GetInventory().GetReplicatedEntries()) FFortItemEntry(ItemAndCount.Item, ItemAndCount.Count, ItemAndCount.Item->GetMinLevel());

        ItemEntry->GetParentInventory() = Inventory;
        ItemEntry->GetIsReplicatedCopy() = true;

        Inventory->SetItemRequiresUpdate(*ItemEntry);

        ItemEntry->GetIsDirty() = true;
    }

    Inventory->HandleInventoryLocalUpdate();
}

void AFortAthenaAIBotController::InitializeInventory(UFortAthenaAIBotInventoryItems* InStartupInventory)
{
    DETOUR(AFortAthenaAIBotController, InitializeInventory, Anchor::Hook()
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Optional()
        .Start()
        .Inline());

    CreateInventory();

    Original(InStartupInventory);

    TArray<FItemAndCount> Items;

    if (auto StartupInventory = GetStartupInventory())
    {
        for (auto ItemDefinition : StartupInventory->GetItems())
            Items.Add({ 1, ItemDefinition });
    }

    if (auto Pickaxe = GetCosmeticLoadoutBC().GetPickaxe())
        Items.Add({ 1, Pickaxe->GetWeaponDefinition() });

    GiveInventory(Items);
}

AFortInventory*& AFortAthenaAIBotController::GetInventory()
{
    static auto Inventory = GetClass()->FindPropertyByName(FName(TEXT("Inventory")));

    return *Inventory->ContainerPtrToValuePtr<AFortInventory*>(this);
}

UFortAthenaAIBotInventoryItems* AFortAthenaAIBotController::GetStartupInventory() const
{
    static auto StartupInventory = GetClass()->FindPropertyByName(FName(TEXT("StartupInventory")));

    return *StartupInventory->ContainerPtrToValuePtr<UFortAthenaAIBotInventoryItems*>(this);
}

FFortAthenaLoadout& AFortAthenaAIBotController::GetCosmeticLoadoutBC()
{
    static auto CosmeticLoadoutBC = GetClass()->FindPropertyByName(FName(TEXT("CosmeticLoadoutBC")));

    return *CosmeticLoadoutBC->ContainerPtrToValuePtr<FFortAthenaLoadout>(this);
}
