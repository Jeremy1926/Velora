#include "pch.h"
#include "Player/FortLiveBroadcastController.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortLiveBroadcastController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortLiveBroadcastController"));

    return Class;
}
