#include "pch.h"
#include "GameFramework/CheatManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

UClass* UCheatManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.CheatManager"));

    return Class;
}
