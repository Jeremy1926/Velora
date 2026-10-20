#pragma once

#include "HAL/Platform.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"

class FField;
class FFieldClass;
class UObject;

class FFieldVariant
{
    union FFieldObjectUnion
    {
        FField* Field;
        UObject* Object;
    } Container;

    bool bIsUObject;
};

class FField
{
    FFieldClass* ClassPrivate;

public:
    virtual ~FField();

    FFieldVariant Owner;
    FField* Next;
    FName NamePrivate;
    EObjectFlags FlagsPrivate;
};
