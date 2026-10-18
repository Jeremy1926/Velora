#include "pch.h"
#include "Abilities/FortHealthSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

const FFortGameplayAttributeData& UFortHealthSet::GetHealth() const
{
    static auto Health = GetClass()->FindPropertyByName(FName(TEXT("Health")));

    return *Health->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}
