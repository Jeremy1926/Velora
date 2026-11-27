#pragma once

#include "UObject/Object.h"

class AActor;
struct FActorComponentTickFunction;

class UActorComponent : public UObject
{
public:
    static UClass* StaticClass();

    AActor* GetOwner() const;

    FActorComponentTickFunction& GetPrimaryComponentTick();
};
