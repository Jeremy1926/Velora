#include "pch.h"
#include "Athena/AI/FortAthenaAIBotInventoryItems.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

TArray<UFortItemDefinition*>& UFortAthenaAIBotInventoryItems::GetItems()
{
    static auto Items = GetClass()->FindPropertyByName(FName(TEXT("Items")));

    return *Items->ContainerPtrToValuePtr<TArray<UFortItemDefinition*>>(this);
}
