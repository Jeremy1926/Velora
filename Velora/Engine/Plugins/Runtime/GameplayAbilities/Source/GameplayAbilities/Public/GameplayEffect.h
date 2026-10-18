#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "GameplayEffectTypes.h"

class UGameplayEffect : public UObject
{
};

struct FGameplayEffectSpec
{
    void SetSetByCallerMagnitude(FName DataName, float Magnitude);
};
