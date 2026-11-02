#pragma once

#include "HAL/Platform.h"

class AController;
class AFortPlayerPawn;
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

struct alignas(8) FInteractionType
{
    FInteractionType(AFortPlayerPawn* InRequestingPawn, ETInteractionType InInteractionType);

    AController* GetController() const;

    ETInteractionType InteractionType;

private:
    uint8 Data[0x7F];
};

class IFortInteractInterface
{
public:
    static UClass* StaticClass();
};
