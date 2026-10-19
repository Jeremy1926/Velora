#pragma once

#include "GameFramework/Actor.h"

class APawn;

class AFortPoiVolume : public AActor
{
public:
    bool OverlapsPawn(APawn* Pawn) const;

    UObject* GetBrushComponent() const;
};
