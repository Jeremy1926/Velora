#pragma once

#include "Delegates/DelegateInstancesImpl.h"

struct FWeakObjectPtr;

template<typename FuncType>
class TDelegate;

template<typename... ParamTypes>
class TDelegate<void(ParamTypes...)> : public FDelegateBase
{
public:
    typedef IBaseDelegateInstance<void(ParamTypes...)> TDelegateInstanceInterface;

    TDelegate()
    {
    }

    TDelegate(const TDelegate& Other)
    {
        *this = Other;
    }

    TDelegate& operator=(const TDelegate& Other)
    {
        if (&Other != this)
        {
            if (auto OtherInstance = (TDelegateInstanceInterface*)Other.GetDelegateInstanceProtected())
                OtherInstance->CreateCopy(*this);
            else
                Unbind();
        }

        return *this;
    }

    template<class UserClass>
    static TDelegate CreateUObject(UserClass* InUserObject, void (UserClass::*InFunc)(ParamTypes...))
    {
        TDelegate Result;

        new (Result) TBaseUObjectMethodDelegateInstance<false, UserClass, void(ParamTypes...)>(InUserObject, InFunc);

        return Result;
    }

    bool IsBound() const
    {
        auto Instance = GetDelegateInstanceProtected();

        return Instance && Instance->IsSafeToExecute();
    }

    bool ExecuteIfBound(ParamTypes... Params) const
    {
        auto Instance = (TDelegateInstanceInterface*)GetDelegateInstanceProtected();

        return Instance && Instance->ExecuteIfSafe(Forward<ParamTypes>(Params)...);
    }
};

template<typename RetValType, typename... ParamTypes>
class TBaseMulticastDelegate;

template<typename... ParamTypes>
class TBaseMulticastDelegate<void, ParamTypes...> : public FMulticastDelegateBase<FWeakObjectPtr>
{
public:
    typedef TDelegate<void(ParamTypes...)> FDelegate;

    template<class UserClass>
    FDelegateHandle AddUObject(UserClass* InUserObject, void (UserClass::*InFunc)(ParamTypes...))
    {
        return AddDelegateInstance(FDelegate::CreateUObject(InUserObject, InFunc));
    }
};

template<typename RetValType, typename... ParamTypes>
class TMulticastDelegate;

template<typename... ParamTypes>
class TMulticastDelegate<void, ParamTypes...> : public TBaseMulticastDelegate<void, ParamTypes...>
{
};
