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
};
