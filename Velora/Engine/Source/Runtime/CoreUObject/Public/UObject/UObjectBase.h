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

    FORCEINLINE uint32 GetUniqueID() const
    {
        return (uint32)InternalIndex;
    }

    FORCEINLINE UClass* GetClass() const
    {
        return ClassPrivate;
    }

    FORCEINLINE UObject* GetOuter() const
    {
        return OuterPrivate;
    }

    FORCEINLINE FName GetFName() const
    {
        return TGameLayout<FName>::Load(&NamePrivate);
    }

    FORCEINLINE EObjectFlags GetFlags() const
    {
        return ObjectFlags;
    }

private:
    EObjectFlags ObjectFlags;
    int32 InternalIndex;
    UClass* ClassPrivate;
    FName NamePrivate;
    UObject* OuterPrivate;
};
