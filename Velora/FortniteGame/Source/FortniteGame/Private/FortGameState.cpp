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

const TArray<FPlaylistStreamedLevelData>& AFortGameState::GetAdditonalPlaylistLevels() const
{
    static auto AdditonalPlaylistLevels = Scanner::String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f")).Call(3).Find("mov *, [rcx+0x*]").Disp(TEXT("AFortGameState::AdditonalPlaylistLevels"));

    return *(const TArray<FPlaylistStreamedLevelData>*)((const uint8*)this + AdditonalPlaylistLevels);
}

bool AFortGameState::AreAllAdditionalPlaylistLevelsLoaded() const
{
    for (auto& LevelData : GetAdditonalPlaylistLevels())
    {
        if (LevelData.StreamingLevel && !LevelData.bIsFinishedStreaming)
            return false;
    }

    return true;
}

bool AFortGameState::AreAllAdditionalPlaylistLevelsVisible() const
{
    static auto AreAllAdditionalPlaylistLevelsVisible = Scanner::Function<bool (*)(const AFortGameState*)>(Scanner::String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f")).Call(3).Get(TEXT("AFortGameState::AreAllAdditionalPlaylistLevelsVisible")));

    return AreAllAdditionalPlaylistLevelsVisible(this);
}

AFortWorldManager* AFortGameState::GetWorldManager() const
{
    static auto WorldManager = GetClass()->FindPropertyByName(FName(TEXT("WorldManager")));

    return *WorldManager->ContainerPtrToValuePtr<AFortWorldManager*>(this);
}

void AFortGameState::StartStreamingAdditionalPlaylistLevel(FName LevelName)
{
    static auto StartStreamingAdditionalPlaylistLevel = Scanner::Function<void (*)(AFortGameState*, FName)>(Scanner::String(TEXT("PLAYLIST: Failed to locate valid level to be streamed %s")).Start().Get(TEXT("AFortGameState::StartStreamingAdditionalPlaylistLevel")));

    StartStreamingAdditionalPlaylistLevel(this, LevelName);
}
