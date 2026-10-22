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
    static auto AdditonalPlaylistLevels = Anchor::Offset(TEXT("AFortGameState::AdditonalPlaylistLevels"))
        .String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f"))
        .Call(3)
        .Find("mov *, [rcx+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f"))
        .Call(3)
        .Find("mov *, [rdi+0x*]")
        .Or()
        .String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f"))
        .Call(3)
        .Find("mov *, [rbx+0x*]")
        .Disp();

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
    static auto AreAllAdditionalPlaylistLevelsVisible = Anchor::Function<bool (*)(const AFortGameState*)>(TEXT("AFortGameState::AreAllAdditionalPlaylistLevelsVisible"))
        .String(TEXT("PLAYLIST: Garbage Collection is being left alone by both Hotfix and Playlist: %s, it remains at %f"))
        .Call(3)
        .Get();

    return AreAllAdditionalPlaylistLevelsVisible(this);
}

AFortWorldManager* AFortGameState::GetWorldManager() const
{
    static auto WorldManager = GetClass()->FindPropertyByName(FName(TEXT("WorldManager")));

    return *WorldManager->ContainerPtrToValuePtr<AFortWorldManager*>(this);
}

void AFortGameState::StartStreamingAdditionalPlaylistLevel(FName LevelName, bool bIsServerOnly)
{
    static auto StartStreamingAdditionalPlaylistLevel = Anchor::Function<void (*)(AFortGameState*, FName, bool)>(TEXT("AFortGameState::StartStreamingAdditionalPlaylistLevel"))
        .String(TEXT("PLAYLIST: Failed to locate valid level to be streamed %s"))
        .Start()
        .Get();

    StartStreamingAdditionalPlaylistLevel(this, LevelName, bIsServerOnly);
}
