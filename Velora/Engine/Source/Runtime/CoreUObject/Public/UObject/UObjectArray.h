#pragma once

#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"

class UObjectBase;

struct FUObjectItem
{
    UObjectBase* Object;
    int32 Flags;
    int32 ClusterRootIndex;
    int32 SerialNumber;

    FORCEINLINE void SetRootSet()
    {
        Flags |= int32(EInternalObjectFlags::RootSet);
    }
};

class FFixedUObjectArray
{
public:
    FUObjectItem* Objects;
    int32 MaxElements;
    int32 NumElements;

    FORCEINLINE int32 Num() const
    {
        return NumElements;
    }

    FORCEINLINE FUObjectItem const& operator[](int32 Index) const
    {
        return Objects[Index];
    }
};

class FUObjectArray
{
public:
    FORCEINLINE FUObjectItem* IndexToObject(int32 Index)
    {
        if (Index < ObjObjects.Num())
            return const_cast<FUObjectItem*>(&ObjObjects[Index]);

        return nullptr;
    }

    typedef FFixedUObjectArray TUObjectArray;

    int32 ObjFirstGCIndex;
    int32 ObjLastNonGCIndex;
    int32 MaxObjectsNotConsideredByGC;
    bool OpenForDisregardForGC;
    TUObjectArray ObjObjects;
};

extern FUObjectArray& GUObjectArray;
