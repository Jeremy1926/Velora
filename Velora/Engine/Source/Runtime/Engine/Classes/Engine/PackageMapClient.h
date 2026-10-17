#pragma once

#include "UObject/Object.h"
#include "UObject/WeakObjectPtr.h"

class FNetGUIDCache
{
public:
    bool SupportsObject(const UObject* Object, const TWeakObjectPtr<UObject>* WeakObjectPtr = nullptr) const;
};
