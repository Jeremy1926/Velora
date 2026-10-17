#pragma once

#include "Containers/UnrealString.h"

class UObject;

struct FStringAssetReference
{
public:
    FORCEINLINE bool IsValid() const
    {
        return !AssetLongPathname.IsEmpty();
    }

    UObject* TryLoad() const;

private:
    FString AssetLongPathname;
};
