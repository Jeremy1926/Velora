#pragma once

#include "UObject/Object.h"
#include "Containers/UnrealString.h"
#include "Misc/NetworkGuid.h"
#include "UObject/Field.h"

class FArchive;
class FFieldNetCache;
class UClass;

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

class FClassNetCache
{
public:
    const FFieldNetCache* GetFromField(FFieldVariant Field) const;
};

class FClassNetCacheMgr
{
public:
    const FClassNetCache* GetClassNetCache(UClass* Class);
};

class FNetBitWriter
{
public:
    static int32 GetTypeSize();

    static void* operator new(size_t Size);

    static void operator delete(void* Ptr);

    FNetBitWriter(UPackageMap* InPackageMap, int64 InMaxBits);

    ~FNetBitWriter();
};
