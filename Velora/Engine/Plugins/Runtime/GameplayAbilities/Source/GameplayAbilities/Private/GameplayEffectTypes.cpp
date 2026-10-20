#include "pch.h"
#include "GameplayEffectTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FScalableFloat::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.ScalableFloat"));

    return Struct;
}

float FScalableFloat::GetValueAtLevel(float Level, const FString* ContextString) const
{
    static auto GetValueAtLevel = Anchor::Function<float (*)(const FScalableFloat*, float, const FString*)>(TEXT("FScalableFloat::GetValueAtLevel"))
        .String(TEXT("FScalableFloat::GetValueAtLevel"))
        .Start()
        .Get();

    return GetValueAtLevel(this, Level, ContextString);
}

bool FGameplayTagCountContainer::UpdateTagMap_Internal(const FGameplayTag& Tag, int32 CountDelta, bool bDeferParentTagsOnRemove)
{
    static auto UpdateTagMap_Internal = Anchor::Function<bool (*)(FGameplayTagCountContainer*, const FGameplayTag&, int32, bool)>(TEXT("FGameplayTagCountContainer::UpdateTagMap_Internal"))
        .String(TEXT("Attempted to remove tag: %s from tag count container, but it is not explicitly in the container!"))
        .Start()
        .Get();

    return UpdateTagMap_Internal(this, Tag, CountDelta, bDeferParentTagsOnRemove);
}
