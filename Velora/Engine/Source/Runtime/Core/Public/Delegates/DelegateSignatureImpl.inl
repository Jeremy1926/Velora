#pragma once

#include "Delegates/DelegateInstancesImpl.h"

struct FWeakObjectPtr;

template<typename FuncType>
class TDelegate;

template<typename... ParamTypes>
class TDelegate<void(ParamTypes...)> : public FDelegateBase
{
public:
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
            if (auto OtherInstance = Other.GetDelegateInstanceProtected())
                IDelegateInstance::WithLayout(OtherInstance, [&](auto, auto bDestructorFirst) { ((IBaseDelegateInstance<void(ParamTypes...), decltype(bDestructorFirst)::value>*)OtherInstance)->CreateCopy(*this); });
            else
                Unbind();
        }

        return *this;
    }

    template<class UserClass>
    static TDelegate CreateUObject(UserClass* InUserObject, void (UserClass::*InFunc)(ParamTypes...))
    {
        TDelegate Result;

        IDelegateInstance::WithLayout(nullptr, [&](auto, auto bDestructorFirst) { new (Result) TBaseUObjectMethodDelegateInstance<false, UserClass, void(ParamTypes...), decltype(bDestructorFirst)::value>(InUserObject, InFunc); });

        return Result;
    }

    template<typename FunctorType>
    static TDelegate CreateLambda(FunctorType&& InFunctor)
    {
        TDelegate Result;

        IDelegateInstance::WithLayout(nullptr, [&](auto, auto bDestructorFirst) { new (Result) TBaseFunctorDelegateInstance<void(ParamTypes...), std::remove_reference_t<FunctorType>, decltype(bDestructorFirst)::value>(InFunctor); });

        return Result;
    }

    bool IsBound() const
    {
        auto Instance = GetDelegateInstanceProtected();

        return Instance && IDelegateInstance::IsSafeToExecute(Instance);
    }

    bool ExecuteIfBound(ParamTypes... Params) const
    {
        auto Instance = GetDelegateInstanceProtected();

        return Instance && IDelegateInstance::WithLayout(Instance, [&](auto, auto bDestructorFirst) { return ((IBaseDelegateInstance<void(ParamTypes...), decltype(bDestructorFirst)::value>*)Instance)->ExecuteIfSafe(Forward<ParamTypes>(Params)...); });
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

    template<typename FunctorType>
    FDelegateHandle AddLambda(FunctorType&& InFunctor)
    {
        return AddDelegateInstance(FDelegate::CreateLambda(Forward<FunctorType>(InFunctor)));
    }

    void Broadcast(ParamTypes... Params) const
    {
        bool NeedsCompaction = false;

        this->LockInvocationList();

        const auto& LocalInvocationList = this->GetInvocationList();

        for (int32 InvocationListIndex = LocalInvocationList.Num() - 1; InvocationListIndex >= 0; --InvocationListIndex)
        {
            auto DelegateInstance = this->GetDelegateInstanceProtectedHelper(LocalInvocationList[InvocationListIndex]);

            if (!DelegateInstance || !IDelegateInstance::WithLayout(DelegateInstance, [&](auto, auto bDestructorFirst) { return ((IBaseDelegateInstance<void(ParamTypes...), decltype(bDestructorFirst)::value>*)DelegateInstance)->ExecuteIfSafe(Params...); }))
                NeedsCompaction = true;
        }

        this->UnlockInvocationList();

        if (NeedsCompaction)
            const_cast<TBaseMulticastDelegate*>(this)->CompactInvocationList();
    }
};

template<typename RetValType, typename... ParamTypes>
class TMulticastDelegate;

template<typename... ParamTypes>
class TMulticastDelegate<void, ParamTypes...> : public TBaseMulticastDelegate<void, ParamTypes...>
{
};
