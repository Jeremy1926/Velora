#include "pch.h"
#include "Athena/AI/AthenaAIDirector.h"
#include "UObject/UObjectGlobals.h"

UClass* AAthenaAIDirector::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaAIDirector"));

    return Class;
}
