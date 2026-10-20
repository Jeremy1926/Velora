#pragma once

#include "AI/Navigation/FortNavSystem.h"

class UAthenaNavSystem : public UFortNavSystem
{
public:
    void ReleaseInitialBuildingLock();
};
