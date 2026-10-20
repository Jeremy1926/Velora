#pragma once

#include "GameFramework/Actor.h"

struct FFrame;

class ANightNightBase : public AActor
{
public:
    void MakePreparationsForNightNight();
    void ReturnToNight();

    void execMakePreparationsForNightNight(FFrame& Stack, void* const Z_Param__Result);
    void execReturnToNight(FFrame& Stack, void* const Z_Param__Result);
};
