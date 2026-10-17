#pragma once

#include "UObject/UObjectBase.h"
#include "UObject/UObjectArray.h"

class UObjectBaseUtility : public UObjectBase
{
public:
    FORCEINLINE void AddToRoot()
    {
        GUObjectArray.IndexToObject((int32)GetUniqueID())->SetRootSet();
    }

    FORCEINLINE bool IsPendingKill() const
    {
        return GUObjectArray.IndexToObject((int32)GetUniqueID())->IsPendingKill();
    }

    bool IsA(const UClass* SomeBase) const;
};
