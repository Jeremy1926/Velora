#include "pch.h"
#include "Pawns/FortPawn.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

AFortWeapon* AFortPawn::GetCurrentWeapon() const
{
    static auto CurrentWeapon = GetClass()->FindPropertyByName(FName(TEXT("CurrentWeapon")));

    return *CurrentWeapon->ContainerPtrToValuePtr<AFortWeapon*>(this);
}
