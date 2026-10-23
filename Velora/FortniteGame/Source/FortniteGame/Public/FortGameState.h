#pragma once

#include "GameFramework/GameStateBase.h"

class AFortWorldManager;
class ULevelStreamingDynamic;
class UScriptStruct;

struct FPlaylistStreamedLevelData
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    bool GetbIsFinishedStreaming() const;

    ULevelStreamingDynamic* StreamingLevel;
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

    const TRuntimeSizedArray<FPlaylistStreamedLevelData>& GetAdditonalPlaylistLevels() const;

    bool AreAllAdditionalPlaylistLevelsLoaded() const;

    bool AreAllAdditionalPlaylistLevelsVisible() const;

    AFortWorldManager* GetWorldManager() const;

    void StartStreamingAdditionalPlaylistLevel(FName LevelName, bool bIsServerOnly);
};
