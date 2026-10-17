#pragma once

#include "FortGameMode.h"

class AFortGameModeZone : public AFortGameMode
{
public:
    typedef AFortGameMode Super;

    void FinishWorldInitialization();
};
