#pragma once

#include "HAL/Platform.h"
#include <new>
#include <type_traits>

namespace UE::Core::Private::Function
{
    struct IFunction_OwnedObject
    {
        virtual void* CloneToEmptyStorage(void* Storage) const = 0;

        virtual void* GetAddress() = 0;

        virtual void Destroy() = 0;

        virtual ~IFunction_OwnedObject() = default;
    };

    struct FFunctionStorage
    {
        IFunction_OwnedObject* GetBoundObject() const
        {
            return (IFunction_OwnedObject*)(HeapAllocation ? HeapAllocation : (void*)InlineAllocation);
        }

        void* HeapAllocation = nullptr;
        alignas(16) uint8 InlineAllocation[0x20];
    };

    template<typename T>
    struct TFunction_OwnedObject_Inline final : public IFunction_OwnedObject
    {
        explicit TFunction_OwnedObject_Inline(const T& InObj)
            : Obj(InObj)
        {
        }

        void* CloneToEmptyStorage(void* Storage) const override
        {
            return &(new(((FFunctionStorage*)Storage)->InlineAllocation) TFunction_OwnedObject_Inline(Obj))->Obj;
        }

        void* GetAddress() override
        {
            return &Obj;
        }

        void Destroy() override
        {
            this->~TFunction_OwnedObject_Inline();
        }

        T Obj;
    };
}

template<typename FuncType>
class TFunction;

template<typename Ret, typename... ParamTypes>
class TFunction<Ret(ParamTypes...)>
{
public:
    TFunction(decltype(nullptr) = nullptr)
    {
    }

    template<typename FunctorType> requires (!std::is_same_v<std::decay_t<FunctorType>, TFunction> && !std::is_same_v<std::decay_t<FunctorType>, decltype(nullptr)>)
    TFunction(FunctorType&& InFunc)
    {
        using OwnedType = UE::Core::Private::Function::TFunction_OwnedObject_Inline<std::decay_t<FunctorType>>;

        static_assert(sizeof(OwnedType) <= sizeof(Storage.InlineAllocation) && alignof(OwnedType) <= alignof(UE::Core::Private::Function::FFunctionStorage));

        new(Storage.InlineAllocation) OwnedType(InFunc);
        Callable = &Call<std::decay_t<FunctorType>>;
    }

    TFunction(const TFunction& Other)
        : Callable(Other.Callable)
    {
        if (Callable)
            Other.Storage.GetBoundObject()->CloneToEmptyStorage(&Storage);
    }

    TFunction(TFunction&& Other)
        : Callable(Other.Callable)
        , Storage(Other.Storage)
    {
        Other.Callable = nullptr;
    }

    ~TFunction()
    {
        if (Callable)
            Storage.GetBoundObject()->Destroy();
    }

    TFunction& operator=(TFunction&& Other)
    {
        if (this != &Other)
        {
            const auto TempCallable = Callable;
            const auto TempStorage = Storage;

            Callable = Other.Callable;
            Storage = Other.Storage;
            Other.Callable = TempCallable;
            Other.Storage = TempStorage;
        }

        return *this;
    }

    explicit operator bool() const
    {
        return Callable != nullptr;
    }

    Ret operator()(ParamTypes... Params) const
    {
        return Callable(Storage.GetBoundObject()->GetAddress(), Params...);
    }

private:
    template<typename FunctorType>
    static Ret Call(void* Obj, ParamTypes&... Params)
    {
        return (*(FunctorType*)Obj)(Params...);
    }

    Ret (*Callable)(void*, ParamTypes&...) = nullptr;
    UE::Core::Private::Function::FFunctionStorage Storage;
};
