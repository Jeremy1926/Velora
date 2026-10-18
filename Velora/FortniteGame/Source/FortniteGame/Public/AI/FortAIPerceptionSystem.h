#pragma once

#include "UObject/Object.h"

class AActor;

class UFortAIPerceptionSystem : public UObject
{
public:
    static void ForceForgetEnemy(AActor* Enemy);
};
