#include "pch.h"
#include "GameplayEffect.h"
#include "Scanner.h"

void FGameplayEffectSpec::SetSetByCallerMagnitude(FName DataName, float Magnitude)
{
    static auto SetSetByCallerMagnitude = Scanner::Function<void (*)(FGameplayEffectSpec*, FName, float)>(Scanner::Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:AssignSetByCallerMagnitude")).Call().Get(TEXT("FGameplayEffectSpec::SetSetByCallerMagnitude")));

    SetSetByCallerMagnitude(this, DataName, Magnitude);
}
