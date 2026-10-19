#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "UObject/AssetPtr.h"

class UBGAConsumableWrapperItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    const TAssetPtr<UClass>& GetConsumableClass() const;
};
