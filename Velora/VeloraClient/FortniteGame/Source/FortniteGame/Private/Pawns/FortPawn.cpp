#include "pch.h"
#include "Pawns/FortPawn.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

UFortMovementSet* AFortPawn::GetMovementSet() const
{
    static auto MovementSet = GetClass()->FindPropertyByName(FName(TEXT("MovementSet")));

    return *MovementSet->ContainerPtrToValuePtr<UFortMovementSet*>(this);
}
