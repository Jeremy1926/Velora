#pragma once

#include "HAL/Platform.h"

class AController;
class UClass;

enum class ETInteractionType : uint8
{
    IT_NoInteraction,
    IT_Simple,
    IT_LongPress,
    IT_BuildingEdit,
    IT_BuildingImprovement,
    IT_TrapPlacement,
    IT_MAX
};

struct FInteractionType
{
    AController* GetController() const;

    ETInteractionType InteractionType;
};

class IFortInteractInterface
{
public:
    static UClass* StaticClass();
};
