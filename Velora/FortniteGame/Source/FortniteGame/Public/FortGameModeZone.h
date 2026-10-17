#pragma once

#include "FortGameMode.h"
#include "Containers/Array.h"

struct FItemAndCount;

class AFortGameModeZone : public AFortGameMode
{
public:
    typedef AFortGameMode Super;

    void FinishWorldInitialization();

    TArray<FItemAndCount>& GetStartingItems();
};
