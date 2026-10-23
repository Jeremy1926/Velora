#include "pch.h"
#include "Building/BuildingGameplayActorSpawnChip.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

uint8 ABuildingGameplayActorSpawnChip::GetSquadId() const
{
    static auto SquadId = GetClass()->FindPropertyByName(FName(TEXT("SquadId")));

    return SquadId ? *SquadId->ContainerPtrToValuePtr<uint8>(this) : 0;
}
