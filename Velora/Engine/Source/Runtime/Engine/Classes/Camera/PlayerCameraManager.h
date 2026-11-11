#pragma once

#include "Containers/EnumAsByte.h"

enum EViewTargetBlendFunction : int
{
    VTBlend_Linear,
    VTBlend_Cubic,
    VTBlend_EaseIn,
    VTBlend_EaseOut,
    VTBlend_PreBlended,
    VTBlend_MAX
};

struct FViewTargetTransitionParams
{
    float BlendTime;
    TEnumAsByte<EViewTargetBlendFunction> BlendFunction;
    float BlendExp;
    uint32 bLockOutgoing : 1;

    FViewTargetTransitionParams()
        : BlendTime(0.f)
        , BlendFunction(VTBlend_Cubic)
        , BlendExp(2.f)
        , bLockOutgoing(false)
    {
    }
};
