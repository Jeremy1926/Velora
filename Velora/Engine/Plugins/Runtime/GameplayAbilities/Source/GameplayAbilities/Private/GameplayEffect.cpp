#include "pch.h"
#include "GameplayEffect.h"
#include "Scanner.h"

void FGameplayEffectSpec::SetSetByCallerMagnitude(FName DataName, float Magnitude)
{
    static auto SetSetByCallerMagnitude = Anchor::Function<void (*)(FGameplayEffectSpec*, FName, float)>(TEXT("FGameplayEffectSpec::SetSetByCallerMagnitude"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:AssignSetByCallerMagnitude"))
        .Call()
        .Get();

    SetSetByCallerMagnitude(this, DataName, Magnitude);
}
