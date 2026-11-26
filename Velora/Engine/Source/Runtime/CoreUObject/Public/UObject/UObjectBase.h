#pragma once

#include "HAL/Platform.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"

class UClass;
class UObject;

class UObjectBase
{
public:
    virtual ~UObjectBase();

    uint32 GetUniqueID() const;

    FORCEINLINE UClass* GetClass() const
    {
        return ClassPrivate;
    }

    FORCEINLINE UObject* GetOuter() const
    {
        return OuterPrivate;
    }

    FName GetFName() const;

    EObjectFlags GetFlags() const;

private:
    EObjectFlags ObjectFlags;
    int32 InternalIndex;
    UClass* ClassPrivate;
    FName NamePrivate;
    UObject* OuterPrivate;
};
