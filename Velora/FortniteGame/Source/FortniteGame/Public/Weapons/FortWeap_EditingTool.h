#pragma once

#include "Weapons/FortWeapon.h"

class ABuildingSMActor;

class AFortWeap_EditingTool : public AFortWeapon
{
public:
    static UClass* StaticClass();

    void SetEditActor(ABuildingSMActor* InEditActor);
};
