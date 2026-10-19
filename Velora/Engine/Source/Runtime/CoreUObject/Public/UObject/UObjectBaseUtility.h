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

    FORCEINLINE FString GetName() const
    {
        return GetFName().ToString();
    }

    bool IsA(const UClass* SomeBase) const;

    void* GetInterfaceAddress(UClass* InterfaceClass);
};

FORCEINLINE FString GetNameSafe(const UObjectBaseUtility* Object)
{
    if (Object == NULL)
    {
        return TEXT("None");
    }
    else
    {
        return Object->GetName();
    }
}
