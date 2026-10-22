#pragma once

#include "HAL/Platform.h"
#include "Misc/Crc.h"

struct FIntVector
{
    int32 X;
    int32 Y;
    int32 Z;

    FORCEINLINE bool operator==(const FIntVector& Other) const
    {
        return X == Other.X && Y == Other.Y && Z == Other.Z;
    }

    FORCEINLINE bool operator!=(const FIntVector& Other) const
    {
        return X != Other.X || Y != Other.Y || Z != Other.Z;
    }
};

FORCEINLINE uint32 GetTypeHash(const FIntVector& Vector)
{
    return FCrc::MemCrc32(&Vector, sizeof(FIntVector));
}
