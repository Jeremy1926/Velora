#include "pch.h"
#include "Athena/FortGameModeAthena.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortGameModeAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeAthena"));

    return Class;
}
