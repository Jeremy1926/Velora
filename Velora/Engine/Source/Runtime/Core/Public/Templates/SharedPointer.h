#pragma once

#include "HAL/Platform.h"
#include "HAL/UnrealMemory.h"
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

    FORCEINLINE TSharedPtr(ObjectType* InObject, SharedPointerInternals::FReferenceControllerBase* InReferenceController)
        : Object(InObject)
        , SharedReferenceCount(InReferenceController)
    {
    }

    FORCEINLINE TSharedPtr(TSharedPtr const& InSharedPtr)
        : Object(InSharedPtr.Object)
        , SharedReferenceCount(InSharedPtr.SharedReferenceCount)
    {
    }

    FORCEINLINE TSharedPtr(TSharedPtr&& InSharedPtr)
        : Object(InSharedPtr.Object)
        , SharedReferenceCount(MoveTemp(InSharedPtr.SharedReferenceCount))
    {
        InSharedPtr.Object = nullptr;
    }

    FORCEINLINE TSharedPtr& operator=(TSharedPtr const& InSharedPtr)
    {
        SharedReferenceCount = InSharedPtr.SharedReferenceCount;
        Object = InSharedPtr.Object;

        return *this;
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

    FORCEINLINE ObjectType& operator*() const
    {
        return *Object;
    }

    FORCEINLINE ObjectType* operator->() const
    {
        return Object;
    }

    FORCEINLINE ObjectType* Get() const
    {
        return Object;
    }

    FORCEINLINE const bool IsValid() const
    {
        return Object != nullptr;
    }

private:
    ObjectType* Object;
    SharedPointerInternals::FSharedReferencer<Mode> SharedReferenceCount;
};

template<class ObjectType, ESPMode Mode>
class TSharedRef
{
public:
    FORCEINLINE TSharedRef()
        : Object(nullptr)
    {
    }

    FORCEINLINE TSharedRef& operator=(TSharedRef&& InSharedRef)
    {
        FMemory::Memswap(this, &InSharedRef, sizeof(TSharedRef));
        return *this;
    }

    FORCEINLINE ObjectType& Get() const
    {
        return *Object;
    }

private:
    ObjectType* Object;
    SharedPointerInternals::FSharedReferencer<Mode> SharedReferenceCount;
};
