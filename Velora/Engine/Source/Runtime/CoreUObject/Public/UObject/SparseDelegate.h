#pragma once

#include "UObject/NameTypes.h"
#include "UObject/ScriptDelegates.h"

class UObject;

struct FSparseDelegate
{
    bool bIsBound;
};

struct FSparseDelegateStorage
{
    static bool Add(const UObject* DelegateOwner, const FName DelegateName, FScriptDelegate Delegate);

    static bool AddUnique(const UObject* DelegateOwner, const FName DelegateName, FScriptDelegate Delegate);

    static bool Remove(const UObject* DelegateOwner, const FName DelegateName, const UObject* ObjectToRemove, const FName FunctionNameToRemove);

    static bool RemoveAll(const UObject* DelegateOwner, const FName DelegateName, const UObject* UserObject);
};
