#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"

class AActor;

class UAIPerceptionComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void GetPerceivedHostileActors(TArray<AActor*>& OutActors) const;
};
