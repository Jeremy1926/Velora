#include "pch.h"
#include "Player/FortPlayerStateZone.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortPlayerStateZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateZone"));

    return Class;
}
