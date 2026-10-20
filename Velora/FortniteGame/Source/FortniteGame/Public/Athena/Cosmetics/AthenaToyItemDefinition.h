#pragma once

#include "Athena/Cosmetics/AthenaDanceItemDefinition.h"
#include "UObject/AssetPtr.h"

class UAthenaToyItemDefinition : public UAthenaDanceItemDefinition
{
public:
    static UClass* StaticClass();

    const TAssetPtr<UClass>* GetToySpawnAbility() const;
};
