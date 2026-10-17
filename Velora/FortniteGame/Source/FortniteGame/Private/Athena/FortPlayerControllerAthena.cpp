#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortPlayerControllerAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerAthena"));

    return Class;
}
