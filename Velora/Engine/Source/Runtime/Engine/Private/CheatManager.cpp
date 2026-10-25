#include "pch.h"
#include "GameFramework/CheatManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UCheatManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.CheatManager"));

    return Class;
}

void UCheatManager::InitCheatManager()
{
    static auto InitCheatManager = Anchor::VirtualFunction<void (*)(UCheatManager*)>(TEXT("UCheatManager::InitCheatManager"))
        .String(TEXT("&UCheatManager::OnPlayerEndPlayed"))
        .Start()
        .SlotIn(TEXT("/Script/Engine.Default__CheatManager"))
        .Or()
        .Event(TEXT("ReceiveInitCheatManager"))
        .InVirtual(TEXT("/Script/Engine.Default__CheatManager"), TEXT("UCheatManager::InitCheatManager"))
        .Get();

    if (InitCheatManager)
        InitCheatManager(this);
}
