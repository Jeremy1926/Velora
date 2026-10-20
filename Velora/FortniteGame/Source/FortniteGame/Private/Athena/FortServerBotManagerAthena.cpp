#include "pch.h"
#include "Athena/FortServerBotManagerAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortServerBotManagerAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortServerBotManagerAthena"));

    return Class;
}

AFortGameModeAthena*& UFortServerBotManagerAthena::GetCachedGameMode()
{
    static auto CachedGameMode = GetClass()->FindPropertyByName(FName(TEXT("CachedGameMode")));

    return *CachedGameMode->ContainerPtrToValuePtr<AFortGameModeAthena*>(this);
}

AFortGameStateAthena*& UFortServerBotManagerAthena::GetCachedGameState()
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);
}
