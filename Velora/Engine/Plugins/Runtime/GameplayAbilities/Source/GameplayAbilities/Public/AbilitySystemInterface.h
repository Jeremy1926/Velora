#pragma once

#include "UObject/Interface.h"

class UAbilitySystemComponent;

class UAbilitySystemInterface : public UInterface
{
public:
    static UClass* StaticClass();
};

class IAbilitySystemInterface
{
protected:
    virtual ~IAbilitySystemInterface()
    {
    }

public:
    typedef UAbilitySystemInterface UClassType;

    virtual UObject* _getUObject() const = 0;

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const = 0;
};
