#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

class ABuildingActor : public AActor
{
public:
    void BeginPlay();

    void SilentDie();

    float GetMaxHealth() const;

    bool HasHealthLeft() const;

    const FGameplayTagContainer& GetStaticGameplayTags() const;

    bool GetDestroyed() const;
};
