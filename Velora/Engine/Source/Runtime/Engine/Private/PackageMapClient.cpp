#include "pch.h"
#include "Engine/PackageMapClient.h"
#include "Scanner.h"

bool FNetGUIDCache::SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr) const
{
    static auto SupportsObject = (bool (*)(const FNetGUIDCache*, const UObject*, const TWeakObjectPtr<UObject>*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindStringReference(TEXT("FNetGUIDCache::SupportsObject: %s NOT Supported.")))->BeginAddress);

    return SupportsObject(this, Object, WeakObjectPtr);
}
