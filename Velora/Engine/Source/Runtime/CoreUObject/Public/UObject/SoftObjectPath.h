#pragma once

#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

class UObject;

struct FSoftObjectPath
{
public:
    FORCEINLINE bool IsValid() const
    {
        return AssetPathName != NAME_None;
    }

    FString ToString() const;

    UObject* TryLoad() const;

private:
    FName AssetPathName;
    FString SubPathString;
};
