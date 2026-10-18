#include "pch.h"
#include "Weapons/FortWeap_BuildingTool.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortWeap_BuildingTool::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeap_BuildingTool"));

    return Class;
}
