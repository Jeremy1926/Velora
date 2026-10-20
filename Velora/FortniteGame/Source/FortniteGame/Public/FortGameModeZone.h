#pragma once

#include "FortGameMode.h"
#include "Containers/Array.h"

class AFortAIDirector;
struct FItemAndCount;

class AFortGameModeZone : public AFortGameMode
{
public:
    typedef AFortGameMode Super;

    static UClass* StaticClass();

    void FinishWorldInitialization();

    void CreateAIDirector();

    AFortAIDirector* GetAIDirector();

    int32 GetOverrideAIDirectorIndex() const;

    TArray<FItemAndCount>& GetStartingItems();
};
