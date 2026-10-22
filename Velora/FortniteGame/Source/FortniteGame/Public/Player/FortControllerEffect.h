#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"

class AController;

struct FEffectRequestContext
{
    AController* InstigatingController = nullptr;
    FGameplayTagContainer SourceTags;
    FGameplayTagContainer TargetTags;
    FGameplayTagContainer ContextTags;
    uint8 Pad[0x10] = {};
};

class UFortControllerEffect : public UObject
{
public:
    static UClass* StaticClass();

    void ApplyEffect(AController* TargetController, const FEffectRequestContext& RequestContext) const;
};

class UFortControllerEffect_ApplyGameplayEffect : public UFortControllerEffect
{
public:
    static UClass* StaticClass();

    void ApplyGameplayEffect(AController* TargetController) const;
};
