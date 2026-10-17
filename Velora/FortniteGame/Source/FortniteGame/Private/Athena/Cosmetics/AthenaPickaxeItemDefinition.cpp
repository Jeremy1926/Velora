#include "pch.h"
#include "Athena/Cosmetics/AthenaPickaxeItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

UFortWeaponMeleeItemDefinition*& UAthenaPickaxeItemDefinition::GetWeaponDefinition()
{
    static auto WeaponDefinition = GetClass()->FindPropertyByName(FName(TEXT("WeaponDefinition")));

    return *WeaponDefinition->ContainerPtrToValuePtr<UFortWeaponMeleeItemDefinition*>(this);
}
