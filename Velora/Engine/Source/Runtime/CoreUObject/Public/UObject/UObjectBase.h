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

    FORCEINLINE UClass* GetClass() const
    {
        return ClassPrivate;
    }

private:
    EObjectFlags ObjectFlags;
    int32 InternalIndex;
    UClass* ClassPrivate;
    FName NamePrivate;
    UObject* OuterPrivate;
};
