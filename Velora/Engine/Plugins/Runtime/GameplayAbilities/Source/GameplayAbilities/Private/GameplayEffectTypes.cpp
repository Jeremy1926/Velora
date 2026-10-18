#include "pch.h"
#include "GameplayEffectTypes.h"
#include "Scanner.h"

float FScalableFloat::GetValueAtLevel(float Level, const FString* ContextString) const
{
    static auto GetValueAtLevel = Scanner::Function<float (*)(const FScalableFloat*, float, const FString*)>(Scanner::String(TEXT("FScalableFloat::GetValueAtLevel")).Start().Get(TEXT("FScalableFloat::GetValueAtLevel")));

    return GetValueAtLevel(this, Level, ContextString);
}

bool FGameplayTagCountContainer::UpdateTagMap_Internal(const FGameplayTag& Tag, int32 CountDelta)
{
    static auto UpdateTagMap_Internal = Scanner::Function<bool (*)(FGameplayTagCountContainer*, const FGameplayTag&, int32)>(Scanner::String(TEXT("Attempted to remove tag: %s from tag count container, but it is not explicitly in the container!")).Start().Get(TEXT("FGameplayTagCountContainer::UpdateTagMap_Internal")));

    return UpdateTagMap_Internal(this, Tag, CountDelta);
}
