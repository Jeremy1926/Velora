#pragma once

#include "Athena/Cosmetics/AthenaCosmeticItemDefinition.h"
#include "Items/FortWeaponMeleeItemDefinition.h"

class UAthenaPickaxeItemDefinition : public UAthenaCosmeticItemDefinition
{
public:
    UFortWeaponMeleeItemDefinition*& GetWeaponDefinition();
};
