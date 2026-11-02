#pragma once

#include <Anchor/Anchor.h>
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
    struct FEncryptedLayout
    {
        Anchor::FCipher Next;
    };

    static const FEncryptedLayout* GetEncryptedLayout();

    static FFieldClass* StaticClass();

    static int32 GetFieldSize();

    FORCEINLINE FField* GetNext() const
    {
        if (auto Layout = GetEncryptedLayout())
            return Layout->Next.Load<FField*>(this);

        return FFieldVariant::IsTaggedPointer() ? ((const FTaggedOwnerField*)this)->Next : Next;
    }

    virtual ~FField();

    FFieldVariant Owner;
    FField* Next;
    FName NamePrivate;
    EObjectFlags FlagsPrivate;
};
