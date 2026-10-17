#pragma once

#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"

class ABuildingActor : public AActor
{
public:
    void BeginPlay();

    void SilentDie();

    const FGameplayTagContainer& GetStaticGameplayTags() const;

    bool GetDestroyed() const;
};
