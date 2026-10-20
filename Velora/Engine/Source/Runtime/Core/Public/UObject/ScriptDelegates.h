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
        InvocationList.AddUnique(InDelegate);
    }

    void Remove(const TScriptDelegate<TWeakPtr>& InDelegate)
    {
        InvocationList.RemoveSingle(InDelegate);
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
