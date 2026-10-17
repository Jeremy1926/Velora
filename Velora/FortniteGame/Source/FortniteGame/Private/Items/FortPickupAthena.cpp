#include "pch.h"
#include "Items/FortPickupAthena.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortPickupAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPickupAthena"));

    return Class;
}
