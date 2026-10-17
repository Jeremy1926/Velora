#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"
#include "Misc/NetworkGuid.h"

class FArchive;

struct FPacketIdRange
{
    FPacketIdRange()
        : First(-1)
        , Last(-1)
    {
    }

    int32 First;
    int32 Last;
};

class UPackageMap : public UObject
{
public:
    bool WriteObject(FArchive& Ar, UObject* InOuter, FNetworkGUID NetGUID, FString ObjName);
};
