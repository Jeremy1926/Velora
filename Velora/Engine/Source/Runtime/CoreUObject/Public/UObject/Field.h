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

    static bool IsTaggedPointer();

    FORCEINLINE UPTRINT ToTaggedPointer() const
    {
        return (UPTRINT)Container.Field | (UPTRINT)bIsUObject;
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

    struct FTaggedOwnerField
    {
        void* VTable;
        FFieldClass* ClassPrivate;
        UPTRINT Owner;
        FField* Next;
        NAME_INDEX NamePrivate;
        EObjectFlags FlagsPrivate;
    };

public:
    static FFieldClass* StaticClass();

    static int32 GetFieldSize();

    static bool IsClassEncoded();

    FFieldClass* GetClass() const;

    FField* GetNext() const;

    virtual ~FField();

    FFieldVariant Owner;
    FField* Next;
    FName NamePrivate;
    EObjectFlags FlagsPrivate;
};
