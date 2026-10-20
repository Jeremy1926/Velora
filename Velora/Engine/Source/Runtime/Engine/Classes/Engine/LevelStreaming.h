#pragma once

#include "UObject/Object.h"

class ULevel;

class ULevelStreaming : public UObject
{
public:
    enum class ECurrentState : uint8
    {
        Removed,
        Unloaded,
        FailedToLoad,
        Loading,
        LoadedNotVisible,
        MakingVisible,
        LoadedVisible,
        MakingInvisible
    };

    ECurrentState GetCurrentState() const;

    ULevel* GetLoadedLevel() const;

    bool IsLevelLoaded() const;

    bool IsLevelVisible() const;

    bool ShouldBeLoaded() const;

    bool ShouldBeVisible() const;
};
