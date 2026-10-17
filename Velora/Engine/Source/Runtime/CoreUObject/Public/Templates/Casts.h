#pragma once

#include "UObject/Object.h"
#include <type_traits>

template<class To, class From>
FORCEINLINE To* Cast(From* Src)
{
    if constexpr (std::is_base_of_v<UObject, To>)
        return Src && Src->IsA(To::StaticClass()) ? (To*)Src : nullptr;
    else
        return Src ? (To*)Src->GetInterfaceAddress(To::UClassType::StaticClass()) : nullptr;
}
