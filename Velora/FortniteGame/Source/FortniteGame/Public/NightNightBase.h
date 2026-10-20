#pragma once

#include "GameFramework/Actor.h"

struct FFrame;

class ANightNightBase : public AActor
{
public:
    void ReturnToNight();

    void execReturnToNight(FFrame& Stack, void* const Z_Param__Result);
};
