#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Misc/NetworkGuid.h"

class FGuidReferences;

using FGuidReferencesMap = TMap<int32, FGuidReferences>;

class FGuidReferences
{
public:
    int32 ParentIndex;
    int32 CmdIndex;
    int32 NumBufferBits;

    TSet<FNetworkGUID> UnmappedGUIDs;
    TSet<FNetworkGUID> MappedDynamicGUIDs;
    TArray<uint8> Buffer;

    FGuidReferencesMap* Array;
};
