#include "pch.h"
#include "Athena/FortAthenaAircraft.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortAthenaAircraft::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAircraft"));

    return Class;
}
