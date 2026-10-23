#pragma once

#include "HAL/Platform.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"

class FField;
class FFieldClass;
class UObject;

class FFieldVariant
{
public:
    FFieldVariant()
        : bIsUObject(false)
    {
        Container.Field = nullptr;
    }

    FFieldVariant(UObject* InObject)
        : bIsUObject(true)
    {
        Container.Object = InObject;
    }

private:
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
    static FFieldClass* StaticClass();

    static int32 GetFieldSize();

    virtual ~FField();

    FFieldVariant Owner;
    FField* Next;
    FName NamePrivate;
    EObjectFlags FlagsPrivate;
};
