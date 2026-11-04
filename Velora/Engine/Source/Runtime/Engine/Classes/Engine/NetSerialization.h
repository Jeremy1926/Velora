#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Misc/NetworkGuid.h"
#include "Misc/EngineVersion.h"
#include "Math/Vector.h"
#include "Net/GuidReferences.h"

struct FFastArraySerializerItem
{
    FFastArraySerializerItem()
        : ReplicationID(-1)
        , ReplicationKey(-1)
        , MostRecentArrayReplicationKey(-1)
    {
    }

    int32 ReplicationID;
    int32 ReplicationKey;
    int32 MostRecentArrayReplicationKey;
};

struct FFastArraySerializerGuidReferences
{
    TSet<FNetworkGUID> UnmappedGUIDs;
    TSet<FNetworkGUID> MappedDynamicGUIDs;
    TArray<uint8> Buffer;
    int32 NumBufferBits;
};

struct FFastArraySerializer
{
    TMap<int32, int32> ItemMap;

    void MarkItemDirty(FFastArraySerializerItem& Item)
    {
        auto& IDCounter = GetIDCounter();

        if (Item.ReplicationID == -1)
        {
            Item.ReplicationID = ++IDCounter;

            if (IDCounter == -1)
                IDCounter++;
        }

        Item.ReplicationKey++;
        MarkArrayDirty();
    }

    void MarkArrayDirty()
    {
        ItemMap.Reset();
        IncrementArrayReplicationKey();

        GetCachedNumItems() = -1;
        GetCachedNumItemsToConsiderForWriting() = -1;
    }

    void IncrementArrayReplicationKey()
    {
        auto& ArrayReplicationKey = GetArrayReplicationKey();

        ArrayReplicationKey++;

        if (ArrayReplicationKey == -1)
            ArrayReplicationKey++;
    }

private:
    int32& GetIDCounter()
    {
        return *(int32*)((uint8*)this + TMap<int32, int32>::GetTypeSize());
    }

    int32& GetArrayReplicationKey()
    {
        return *(&GetIDCounter() + 1);
    }

    int32& GetCachedNumItems()
    {
        auto& Version = FEngineVersion::Current();

        return *(int32*)((uint8*)(&GetArrayReplicationKey() + 1) + (Version.GetMajor() > 4 || Version.GetMinor() >= 23 ? 2 : 1) * TMap<int32, FFastArraySerializerGuidReferences>::GetTypeSize());
    }

    int32& GetCachedNumItemsToConsiderForWriting()
    {
        return *(&GetCachedNumItems() + 1);
    }
};

struct FVector_NetQuantize : public FVector
{
};

struct FVector_NetQuantize10 : public FVector
{
};

struct FVector_NetQuantize100 : public FVector
{
};

struct FVector_NetQuantizeNormal : public FVector
{
};
