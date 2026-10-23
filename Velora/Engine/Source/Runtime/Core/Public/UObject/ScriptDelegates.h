#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class UObject;

template<typename TWeakPtr = FWeakObjectPtr>
class TScriptDelegate
{
public:
    TScriptDelegate()
        : Object()
        , FunctionName(NAME_None)
    {
    }

    void BindUFunction(UObject* InObject, const FName& InFunctionName)
    {
        Object = InObject;
        FunctionName = InFunctionName;
    }

    bool operator==(const TScriptDelegate& Other) const
    {
        return Object == Other.Object && FunctionName == Other.FunctionName;
    }

    bool IsBoundToObject(const UObject* InUserObject) const
    {
        return InUserObject && InUserObject == Object.Get();
    }

    bool IsCompactable() const
    {
        return FunctionName == NAME_None || !Object.Get(true);
    }

    struct FOutlineNumberLayout
    {
        TWeakPtr Object;
        NAME_INDEX FunctionName;

        bool operator==(const FOutlineNumberLayout& Other) const
        {
            return Object == Other.Object && FunctionName == Other.FunctionName;
        }

        bool IsBoundToObject(const UObject* InUserObject) const
        {
            return InUserObject && InUserObject == Object.Get();
        }

        bool IsCompactable() const
        {
            return FunctionName == 0 || !Object.Get(true);
        }
    };

    FOutlineNumberLayout ToOutlineNumberLayout() const
    {
        FOutlineNumberLayout Layout{ Object, 0 };
        TGameLayout<FName>::Store(&Layout.FunctionName, FunctionName);

        return Layout;
    }

protected:
    TWeakPtr Object;

    FName FunctionName;
};

template<typename TWeakPtr = FWeakObjectPtr>
class TMulticastScriptDelegate
{
public:
    void AddUnique(const TScriptDelegate<TWeakPtr>& InDelegate)
    {
        if (FName::IsOutlineNumber())
            ((TArray<typename TScriptDelegate<TWeakPtr>::FOutlineNumberLayout>&)InvocationList).AddUnique(InDelegate.ToOutlineNumberLayout());
        else
            InvocationList.AddUnique(InDelegate);
    }

    void Remove(const TScriptDelegate<TWeakPtr>& InDelegate)
    {
        if (FName::IsOutlineNumber())
            ((TArray<typename TScriptDelegate<TWeakPtr>::FOutlineNumberLayout>&)InvocationList).RemoveSingle(InDelegate.ToOutlineNumberLayout());
        else
            InvocationList.RemoveSingle(InDelegate);
    }

    void RemoveAll(const UObject* Object)
    {
        auto RemoveAllBindings = [Object](auto& Bindings)
        {
            for (int32 BindingIndex = Bindings.Num() - 1; BindingIndex >= 0; --BindingIndex)
            {
                const auto& Binding = Bindings[BindingIndex];

                if (Binding.IsBoundToObject(Object) || Binding.IsCompactable())
                    Bindings.RemoveAt(BindingIndex);
            }
        };

        if (FName::IsOutlineNumber())
            RemoveAllBindings((TArray<typename TScriptDelegate<TWeakPtr>::FOutlineNumberLayout>&)InvocationList);
        else
            RemoveAllBindings(InvocationList);
    }

    template<class UObjectTemplate>
    void ProcessMulticastDelegate(void* Parameters) const;

protected:
    mutable TArray<TScriptDelegate<TWeakPtr>> InvocationList;
};

template<>
template<>
void TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>(void* Parameters) const;

typedef TScriptDelegate<> FScriptDelegate;

typedef TMulticastScriptDelegate<> FMulticastScriptDelegate;
