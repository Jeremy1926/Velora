#pragma once

#include "HAL/Platform.h"

class UObject;

class FDelegateHandle
{
public:
    enum EGenerateNewHandleType
    {
        GenerateNewHandle
    };

    FDelegateHandle()
        : ID(0)
    {
    }

    explicit FDelegateHandle(EGenerateNewHandleType)
        : ID(GenerateNewID())
    {
    }

    bool IsValid() const
    {
        return ID != 0;
    }

    void Reset()
    {
        ID = 0;
    }

private:
    static uint64 GenerateNewID();

    uint64 ID;
};

class IDelegateInstance
{
public:
    virtual ~IDelegateInstance() = default;

    virtual UObject* GetUObject() const = 0;

    virtual const void* GetObjectForTimerManager() const = 0;

    virtual uint64 GetBoundProgramCounterForTimerManager() const = 0;

    virtual bool HasSameObject(const void* InUserObject) const = 0;

    virtual bool IsCompactable() const
    {
        return !IsSafeToExecute();
    }

    virtual bool IsSafeToExecute() const = 0;

    virtual FDelegateHandle GetHandle() const = 0;
};
