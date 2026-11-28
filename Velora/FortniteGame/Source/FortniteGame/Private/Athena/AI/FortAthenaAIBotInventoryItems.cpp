#include "pch.h"
#include "Athena/AI/FortAthenaAIBotInventoryItems.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAthenaAIBotInventoryItems::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIBotInventoryItems"));

    return Class;
}

TArray<FItemAndCount> UFortAthenaAIBotInventoryItems::GetItems(const TArray<FItemAndCount>& InItems)
{
    static auto Items = (UArrayProperty*)StaticClass()->FindPropertyByName(FName(TEXT("Items")));

    if (Items->GetInner()->GetSize() == sizeof(FItemAndCount))
        return InItems;

    TArray<FItemAndCount> ItemsAndCounts;

    if (Items->GetInner()->GetSize() == sizeof(UFortItemDefinition*))
    {
        for (auto ItemDefinition : *(const TArray<UFortItemDefinition*>*)&InItems)
            ItemsAndCounts.Add({ 1, ItemDefinition });

        return ItemsAndCounts;
    }

    auto& Entries = *(const TArray<uint8>*)&InItems;

    for (int32 Index = 0; Index < Entries.Num(); Index++)
        ItemsAndCounts.Add(*(const FItemAndCount*)(Entries.GetData() + Index * Items->GetInner()->GetSize()));

    return ItemsAndCounts;
}

TArray<FItemAndCount> UFortAthenaAIBotInventoryItems::GetItems()
{
    static auto Items = GetClass()->FindPropertyByName(FName(TEXT("Items")));

    return GetItems(*Items->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this));
}
