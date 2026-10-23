#include "pch.h"
#include "Engine/PackageMapClient.h"
#include "Scanner.h"

bool FNetGUIDCache::SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr) const
{
    static auto SupportsObject = Anchor::Function<bool (*)(const FNetGUIDCache*, const UObject*, const TWeakObjectPtr<UObject>*)>(TEXT("FNetGUIDCache::SupportsObject"))
        .String(TEXT("FNetGUIDCache::SupportsObject: %s NOT Supported."))
        .Start()
        .Get();

    return SupportsObject(this, Object, WeakObjectPtr);
}

FNetworkGUID FNetGUIDCache::GetNetGUID(const UObject* Object) const
{
    static auto GetNetGUID = Anchor::Function<FNetworkGUID* (*)(const FNetGUIDCache*, FNetworkGUID*, const UObject*)>(TEXT("FNetGUIDCache::GetNetGUID"))
        .Pattern("48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 57 48 83 EC 20 48 8B F9 4C 8B C2 48 8B 49 ? 48 8D 54 24 38 33 DB 48 8B 89 ? ? ? ? E8")
        .Find("call 0x*")
        .Target()
        .Get();

    FNetworkGUID NetGUID;
    GetNetGUID(this, &NetGUID, Object);

    return NetGUID;
}
