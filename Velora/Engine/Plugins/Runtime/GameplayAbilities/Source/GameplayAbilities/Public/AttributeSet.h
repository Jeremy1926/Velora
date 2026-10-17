#pragma once

#include "Containers/UnrealString.h"

class UProperty;
class UStruct;

struct FGameplayAttribute
{
    FGameplayAttribute(UProperty* NewProperty);

    FString AttributeName;

private:
    UProperty* Attribute;
    UStruct* AttributeOwner;
};
