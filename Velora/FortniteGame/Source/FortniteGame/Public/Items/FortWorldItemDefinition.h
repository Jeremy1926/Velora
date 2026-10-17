#pragma once

#include "Items/FortItemDefinition.h"

class UFortWorldItem;
class AFortPlayerController;

class UFortWorldItemDefinition : public UFortItemDefinition
{
public:
    static UClass* StaticClass();

    bool ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const;
};
