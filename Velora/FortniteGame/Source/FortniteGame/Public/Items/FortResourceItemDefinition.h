#pragma once

#include "Items/FortWorldItemDefinition.h"

enum class EFortResourceType : uint8
{
    Wood,
    Stone,
    Metal,
    Permanite,
    None,
};

class UFortResourceItemDefinition : public UFortWorldItemDefinition
{
};
