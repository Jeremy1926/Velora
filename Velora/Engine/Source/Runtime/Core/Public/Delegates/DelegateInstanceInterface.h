#pragma once

#include "Delegates/IDelegateInstance.h"

class FDelegateBase;

template<typename FuncType>
struct IBaseDelegateInstance;

template<typename... ParamTypes>
struct IBaseDelegateInstance<void(ParamTypes...)> : public IDelegateInstance
{
    virtual void CreateCopy(FDelegateBase& Base) = 0;

    virtual void Execute(ParamTypes... Params) const = 0;

    virtual bool ExecuteIfSafe(ParamTypes... Params) const = 0;
};
