#pragma once

#include "UObject/Object.h"
#include "GameplayTagContainer.h"

class AActor;
class AController;
class UScriptStruct;

struct alignas(8) FEffectRequestContext
{
    FEffectRequestContext();

    ~FEffectRequestContext();

    static UScriptStruct* StaticStruct();

    AController*& GetInstigatingController();

    void SetInstigatingActor(AActor* InstigatingActor);

private:
    uint8 Data[0x80];
};

class UFortControllerEffect : public UObject
{
public:
    static UClass* StaticClass();

    void ApplyEffect(AController* TargetController, const FEffectRequestContext& RequestContext) const;
};

class UFortControllerEffect_ApplyGameplayEffectBase : public UFortControllerEffect
{
public:
    static UClass* StaticClass();

    void ApplyGameplayEffect(AController* TargetController) const;
};

class UFortControllerEffect_ApplyGameplayEffect : public UFortControllerEffect_ApplyGameplayEffectBase
{
public:
    static UClass* StaticClass();
};

class UFortControllerEffect_NPCGiveGift : public UFortControllerEffect
{
public:
    static UClass* StaticClass();

    void GiveGift(AController* TargetController, const FEffectRequestContext& RequestContext) const;
};

class UFortControllerEffect_TriggerCustomFeedMessage : public UFortControllerEffect
{
public:
    static UClass* StaticClass();

    void TriggerCustomFeedMessage(AController* TargetController) const;
};
