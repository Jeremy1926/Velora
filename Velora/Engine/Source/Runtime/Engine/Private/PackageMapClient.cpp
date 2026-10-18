#include "pch.h"
#include "Engine/PackageMapClient.h"
#include "Scanner.h"

bool FNetGUIDCache::SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr) const
{
    static auto SupportsObject = Scanner::Function<bool (*)(const FNetGUIDCache*, const UObject*, const TWeakObjectPtr<UObject>*)>(Scanner::String(TEXT("FNetGUIDCache::SupportsObject: %s NOT Supported.")).Start().Get(TEXT("FNetGUIDCache::SupportsObject")));

    return SupportsObject(this, Object, WeakObjectPtr);
}
