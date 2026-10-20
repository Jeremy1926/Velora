#include "pch.h"
#include "AI/FortAIDirector.h"
#include "UObject/UObjectGlobals.h"

UClass* AFortAIDirector::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIDirector"));

    return Class;
}
