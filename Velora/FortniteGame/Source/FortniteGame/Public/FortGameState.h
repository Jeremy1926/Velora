#pragma once

#include "GameFramework/GameStateBase.h"

class AFortWorldManager;
class ULevelStreamingDynamic;

struct FPlaylistStreamedLevelData
{
    ULevelStreamingDynamic* StreamingLevel;
    FName LevelAssetPathName;
    bool bIsFinishedStreaming;
};

enum class EFortGameplayState : uint8
{
    NormalGameplay,
    WaitingToStart,
    EndOfZone,
    EnteringZone,
    LeavingZone,
    Invalid
};

class AFortGameState : public AGameStateBase
{
public:
    static UClass* StaticClass();

    EFortGameplayState GetGameplayState() const;

    const TArray<FPlaylistStreamedLevelData>& GetAdditonalPlaylistLevels() const;

    bool AreAllAdditionalPlaylistLevelsLoaded() const;

    bool AreAllAdditionalPlaylistLevelsVisible() const;

    AFortWorldManager* GetWorldManager() const;

    void StartStreamingAdditionalPlaylistLevel(FName LevelName, bool bIsServerOnly);
};
