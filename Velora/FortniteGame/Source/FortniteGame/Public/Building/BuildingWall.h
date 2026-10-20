#pragma once

#include "Building/BuildingSMActor.h"

class AController;
class AFortPawn;

enum class EDoorOpenStyle : uint8
{
    Open,
    SlamOpen,
    SmashOpen,
    Close,
    EDoorOpenStyle_MAX
};

class ABuildingWall : public ABuildingSMActor
{
public:
    static UClass* StaticClass();

    void Open_Implementation(AController* ControllerInstigator);

    void Close_Implementation(AController* ControllerInstigator);

    void SetIsDoorOpen(EDoorOpenStyle InDoorOpenStyle, AFortPawn* InteractingPawn);
};
