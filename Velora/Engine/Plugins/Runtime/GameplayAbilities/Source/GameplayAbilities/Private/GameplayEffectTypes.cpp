#include "pch.h"
#include "GameplayEffectTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const FHitResult* FGameplayEffectContext::GetHitResult() const
{
    static auto GetHitResult = Anchor::VirtualFunction<const FHitResult* (*)(const FGameplayEffectContext*)>(TEXT("FGameplayEffectContext::GetHitResult"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EffectContextGetHitResult"))
        .Find("call [rax+0x*]")
        .Get();

    return GetHitResult(this);
}

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
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 65 48 8B 04 25 58 00 00 00 48 8D 35 ? ? ? ? 48 8B F9 0F 29 74 24 30 B9 ? ? ? ? 49 8B D8 0F 28 F1")
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
