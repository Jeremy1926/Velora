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
    int32 IDCounter;
    int32 ArrayReplicationKey;

    TMap<int32, FFastArraySerializerGuidReferences> GuidReferencesMap;

    void MarkItemDirty(FFastArraySerializerItem& Item)
    {
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

        auto& Version = FEngineVersion::Current();
        auto& ArraySerializer = Version.GetMajor() > 4 || Version.GetMinor() >= 23 ? *(FFastArraySerializer*)((uint8*)this + sizeof(TMap<int32, FGuidReferencesMap>)) : *this;

        ArraySerializer.CachedNumItems = -1;
        ArraySerializer.CachedNumItemsToConsiderForWriting = -1;
    }

    void IncrementArrayReplicationKey()
    {
        ArrayReplicationKey++;

        if (ArrayReplicationKey == -1)
            ArrayReplicationKey++;
    }

private:
    int32 CachedNumItems;
    int32 CachedNumItemsToConsiderForWriting;
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
