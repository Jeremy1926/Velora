#include "pch.h"
#include "Items/FortQuickBars.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortQuickBars::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuickBars"));

    return Class;
}
