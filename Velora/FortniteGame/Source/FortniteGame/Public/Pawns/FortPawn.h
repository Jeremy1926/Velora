#pragma once

#include "GameFramework/Character.h"

class AFortWeapon;

class AFortPawn : public ACharacter
{
public:
    AFortWeapon* GetCurrentWeapon() const;
};
