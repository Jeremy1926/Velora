#pragma once

#include "Items/FortMontageItemDefinitionBase.h"

class UAthenaDanceItemDefinition : public UFortMontageItemDefinitionBase
{
public:
    static UClass* StaticClass();

    bool GetMovingEmote() const;

    bool GetMoveForwardOnly() const;
};
