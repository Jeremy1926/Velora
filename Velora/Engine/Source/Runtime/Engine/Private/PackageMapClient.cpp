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
