#pragma once

#include "UObject/Object.h"

template<class To, class From>
FORCEINLINE To* Cast(From* Src)
{
    return Src && Src->IsA(To::StaticClass()) ? (To*)Src : nullptr;
}
