#pragma once

#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"
#include "Misc/NetworkGuid.h"

class FNetGUIDCache
{
public:
    bool SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr = nullptr) const;

    FNetworkGUID GetNetGUID(const UObject* Object) const;
};
