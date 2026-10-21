#pragma once

#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"

class AActor;

class UFortAthenaLivingWorldManager : public UActorComponent
{
public:
    static UClass* StaticClass();

    static int32 GetEventInstancesOffset();

    static void LivingWorldManagerRegisterPointProvider(AActor* PointProvider);

    bool IsLivingWorldEnabled();

    FScalableFloat& GetLagerEnabled();

    TArray<AActor*>& GetPointProviders();
};
