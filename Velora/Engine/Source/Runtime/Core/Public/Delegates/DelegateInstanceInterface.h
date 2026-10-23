#pragma once

#include "Delegates/IDelegateInstance.h"

class FDelegateBase;

template<typename FuncType, bool bDestructorFirst>
struct IBaseDelegateInstance;

template<bool bDestructorFirst, typename RetValType, typename... ParamTypes>
struct IBaseDelegateInstance<RetValType(ParamTypes...), bDestructorFirst> : public TDelegateInstanceLayout<bDestructorFirst>
{
    virtual void CreateCopy(FDelegateBase& Base) = 0;

    virtual RetValType Execute(ParamTypes... Params) const = 0;

    virtual bool ExecuteIfSafe(ParamTypes... Params) const = 0;
};
