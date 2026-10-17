#include "pch.h"
#include "Items/FortItemDefinition.h"
#include "UObject/UnrealType.h"

int32 UFortItemDefinition::GetMinLevel() const
{
    static auto MinLevel = GetClass()->FindPropertyByName(FName(TEXT("MinLevel")));

    return *MinLevel->ContainerPtrToValuePtr<int32>(this);
}
