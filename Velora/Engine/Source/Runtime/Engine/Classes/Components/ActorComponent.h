#pragma once

#include "UObject/Object.h"

class AActor;
struct FActorComponentTickFunction;

class UActorComponent : public UObject
{
public:
    static UClass* StaticClass();

    AActor* GetOwner() const;

    void RegisterComponentTickFunctions(bool bRegister);

    FActorComponentTickFunction& GetPrimaryComponentTick();
};
