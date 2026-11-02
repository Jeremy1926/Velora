#pragma once

#include "Delegates/IDelegateInstance.h"

class FDelegateBase;

template<typename FuncType, int32 Layout>
struct IBaseDelegateInstance;

template<int32 Layout, typename RetValType, typename... ParamTypes>
struct IBaseDelegateInstance<RetValType(ParamTypes...), Layout> : public TDelegateInstanceLayout<Layout != 0>
{
    virtual void CreateCopy(FDelegateBase& Base) = 0;

    virtual RetValType Execute(ParamTypes... Params) const = 0;

    virtual bool ExecuteIfSafe(ParamTypes... Params) const = 0;
};

template<typename RetValType, typename... ParamTypes>
struct IBaseDelegateInstance<RetValType(ParamTypes...), 2> : public TDelegateInstanceLayout<true>
{
    virtual void CreateCopy(FDelegateBase& Base) = 0;

    virtual void CreateCopyNotThreadSafe(FDelegateBase& Base)
    {
        CreateCopy(Base);
    }

    virtual void CreateCopyThreadSafe(CRITICAL_SECTION& Base)
    {
        EnterCriticalSection(&Base);
        CreateCopy(*(FDelegateBase*)(&Base + 1));
        LeaveCriticalSection(&Base);
    }

    virtual RetValType Execute(ParamTypes... Params) const = 0;

    virtual bool ExecuteIfSafe(ParamTypes... Params) const = 0;
};
