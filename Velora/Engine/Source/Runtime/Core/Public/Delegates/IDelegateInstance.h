#pragma once

#include <type_traits>
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

template<bool bDestructorFirst>
class TDelegateInstanceLayout;

template<>
class TDelegateInstanceLayout<false>
{
public:
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

    virtual ~TDelegateInstanceLayout() = default;
};

template<>
class TDelegateInstanceLayout<true>
{
public:
    virtual ~TDelegateInstanceLayout() = default;

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

class IDelegateInstance
{
public:
    static bool IsDestructorFirst();

    template<typename CallableType>
    static decltype(auto) WithLayout(const void* Instance, CallableType&& Callable)
    {
        if (IsDestructorFirst())
            return Callable((TDelegateInstanceLayout<true>*)Instance, std::true_type());

        return Callable((TDelegateInstanceLayout<false>*)Instance, std::false_type());
    }

    static void Destroy(void* Instance)
    {
        WithLayout(Instance, [](auto Interface, auto)
        {
            using InterfaceType = std::remove_pointer_t<decltype(Interface)>;

            Interface->~InterfaceType();
        });
    }

    static bool IsCompactable(const void* Instance)
    {
        return WithLayout(Instance, [](auto Interface, auto) { return Interface->IsCompactable(); });
    }

    static bool IsSafeToExecute(const void* Instance)
    {
        return WithLayout(Instance, [](auto Interface, auto) { return Interface->IsSafeToExecute(); });
    }

    static FDelegateHandle GetHandle(const void* Instance)
    {
        return WithLayout(Instance, [](auto Interface, auto) { return Interface->GetHandle(); });
    }
};
