#include "pch.h"
#include "Athena/Cosmetics/AthenaSprayItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaSprayItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaSprayItemDefinition"));

    return Class;
}
