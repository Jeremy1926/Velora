#pragma once

#include "Components/ActorComponent.h"

class AActor;

class UFortPawnComponent_Disguise : public UActorComponent
{
public:
    static UClass* StaticClass();

    void RevealDisguise(const AActor* RevealInstigator);
};
