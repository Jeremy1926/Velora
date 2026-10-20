#pragma once

#include "UObject/Object.h"

class AActor;

class UActorComponent : public UObject
{
public:
    AActor* GetOwner() const;
};
