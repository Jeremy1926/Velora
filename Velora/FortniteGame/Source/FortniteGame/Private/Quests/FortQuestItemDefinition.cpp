#include "pch.h"
#include "Quests/FortQuestItemDefinition.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortQuestItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuestItemDefinition"));

    return Class;
}
