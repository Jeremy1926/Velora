#pragma once

#include "HAL/Platform.h"
#include "Templates/SharedPointerInternals.h"
#include "Templates/UnrealTemplate.h"

template<class ObjectType, ESPMode Mode>
class TSharedPtr
{
public:
    FORCEINLINE TSharedPtr()
        : Object(nullptr)
    {
    }

    FORCEINLINE TSharedPtr(TSharedPtr&& InSharedPtr)
        : Object(InSharedPtr.Object)
        , SharedReferenceCount(MoveTemp(InSharedPtr.SharedReferenceCount))
    {
        InSharedPtr.Object = nullptr;
    }

    FORCEINLINE TSharedPtr& operator=(TSharedPtr&& InSharedPtr)
    {
        if (this != &InSharedPtr)
        {
            Object = InSharedPtr.Object;
            InSharedPtr.Object = nullptr;
            SharedReferenceCount = MoveTemp(InSharedPtr.SharedReferenceCount);
        }

        return *this;
    }

    FORCEINLINE ObjectType* operator->() const
    {
        return Object;
    }

private:
    ObjectType* Object;
    SharedPointerInternals::FSharedReferencer<Mode> SharedReferenceCount;
};
