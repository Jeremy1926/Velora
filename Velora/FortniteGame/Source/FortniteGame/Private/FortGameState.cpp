#include "pch.h"
#include "FortGameState.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortGameState::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameState"));

    return Class;
}

EFortGameplayState AFortGameState::GetGameplayState() const
{
    static auto GameplayState = GetClass()->FindPropertyByName(FName(TEXT("GameplayState")));

    return *GameplayState->ContainerPtrToValuePtr<EFortGameplayState>(this);
}

void AFortGameState::StartStreamingAdditionalPlaylistLevel(FName LevelName)
{
    static auto StartStreamingAdditionalPlaylistLevel = Scanner::Function<void (*)(AFortGameState*, FName)>(Scanner::String(TEXT("PLAYLIST: Failed to locate valid level to be streamed %s")).Start().Get(TEXT("AFortGameState::StartStreamingAdditionalPlaylistLevel")));

    StartStreamingAdditionalPlaylistLevel(this, LevelName);
}
