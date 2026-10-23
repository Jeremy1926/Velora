#pragma once

#include "Delegates/DelegateBase.h"
#include "Delegates/DelegateInstanceInterface.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/WeakObjectPtr.h"

template<bool bConst, class UserClass, typename FuncType, bool bDestructorFirst>
class TBaseUObjectMethodDelegateInstance;

template<class UserClass, bool bDestructorFirst, typename RetValType, typename... ParamTypes>
class TBaseUObjectMethodDelegateInstance<false, UserClass, RetValType(ParamTypes...), bDestructorFirst> : public IBaseDelegateInstance<RetValType(ParamTypes...), bDestructorFirst>
{
public:
    typedef RetValType (UserClass::*FMethodPtr)(ParamTypes...);

    TBaseUObjectMethodDelegateInstance(UserClass* InUserObject, FMethodPtr InMethodPtr)
        : Handle(FDelegateHandle::GenerateNewHandle)
        , UserObject(InUserObject)
        , MethodPtr(InMethodPtr)
    {
    }

    UObject* GetUObject() const override
    {
        return (UObject*)UserObject.Get();
    }

    const void* GetObjectForTimerManager() const override
    {
        return UserObject.Get();
    }

    uint64 GetBoundProgramCounterForTimerManager() const override
    {
        return *(const uint64*)&MethodPtr;
    }

    bool HasSameObject(const void* InUserObject) const override
    {
        return UserObject.Get() == InUserObject;
    }

    bool IsSafeToExecute() const override
    {
        return !!UserObject.Get();
    }

    FDelegateHandle GetHandle() const override
    {
        return Handle;
    }

    void CreateCopy(FDelegateBase& Base) override
    {
        new (Base) TBaseUObjectMethodDelegateInstance(*this);
    }

    RetValType Execute(ParamTypes... Params) const override
    {
        return (UserObject.Get()->*MethodPtr)(Forward<ParamTypes>(Params)...);
    }

    bool ExecuteIfSafe(ParamTypes... Params) const override
    {
        if (auto ActualUserObject = UserObject.Get())
        {
            (ActualUserObject->*MethodPtr)(Forward<ParamTypes>(Params)...);

            return true;
        }

        return false;
    }

private:
    FDelegateHandle Handle;
    TWeakObjectPtr<UserClass> UserObject;
    FMethodPtr MethodPtr;
};

template<typename FuncType, typename FunctorType, bool bDestructorFirst>
class TBaseFunctorDelegateInstance;

template<typename FunctorType, bool bDestructorFirst, typename RetValType, typename... ParamTypes>
class TBaseFunctorDelegateInstance<RetValType(ParamTypes...), FunctorType, bDestructorFirst> : public IBaseDelegateInstance<RetValType(ParamTypes...), bDestructorFirst>
{
public:
    explicit TBaseFunctorDelegateInstance(const FunctorType& InFunctor)
        : Handle(FDelegateHandle::GenerateNewHandle)
        , Functor(InFunctor)
    {
    }

    UObject* GetUObject() const override
    {
        return nullptr;
    }

    const void* GetObjectForTimerManager() const override
    {
        return nullptr;
    }

    uint64 GetBoundProgramCounterForTimerManager() const override
    {
        return 0;
    }

    bool HasSameObject(const void* InUserObject) const override
    {
        return false;
    }

    bool IsSafeToExecute() const override
    {
        return true;
    }

    FDelegateHandle GetHandle() const override
    {
        return Handle;
    }

    void CreateCopy(FDelegateBase& Base) override
    {
        new (Base) TBaseFunctorDelegateInstance(*this);
    }

    RetValType Execute(ParamTypes... Params) const override
    {
        return Functor(Forward<ParamTypes>(Params)...);
    }

    bool ExecuteIfSafe(ParamTypes... Params) const override
    {
        Functor(Forward<ParamTypes>(Params)...);

        return true;
    }

private:
    FDelegateHandle Handle;
    mutable FunctorType Functor;
};

template<typename FuncType, bool bDestructorFirst>
class TBaseStaticDelegateInstance;

template<bool bDestructorFirst, typename RetValType, typename... ParamTypes>
class TBaseStaticDelegateInstance<RetValType(ParamTypes...), bDestructorFirst> : public IBaseDelegateInstance<RetValType(ParamTypes...), bDestructorFirst>
{
public:
    typedef RetValType (*FFuncPtr)(ParamTypes...);

    explicit TBaseStaticDelegateInstance(FFuncPtr InStaticFuncPtr)
        : Handle(FDelegateHandle::GenerateNewHandle)
        , StaticFuncPtr(InStaticFuncPtr)
    {
    }

    UObject* GetUObject() const override
    {
        return nullptr;
    }

    const void* GetObjectForTimerManager() const override
    {
        return nullptr;
    }

    uint64 GetBoundProgramCounterForTimerManager() const override
    {
        return (uint64)StaticFuncPtr;
    }

    bool HasSameObject(const void* InUserObject) const override
    {
        return false;
    }

    bool IsSafeToExecute() const override
    {
        return true;
    }

    FDelegateHandle GetHandle() const override
    {
        return Handle;
    }

    void CreateCopy(FDelegateBase& Base) override
    {
        new (Base) TBaseStaticDelegateInstance(*this);
    }

    RetValType Execute(ParamTypes... Params) const override
    {
        return StaticFuncPtr(Forward<ParamTypes>(Params)...);
    }

    bool ExecuteIfSafe(ParamTypes... Params) const override
    {
        StaticFuncPtr(Forward<ParamTypes>(Params)...);

        return true;
    }

private:
    FDelegateHandle Handle;
    FFuncPtr StaticFuncPtr;
};
