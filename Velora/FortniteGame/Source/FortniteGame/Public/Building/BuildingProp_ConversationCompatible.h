#pragma once

#include "Building/BuildingActor.h"

struct FInteractionType;

class ABuildingProp_ConversationCompatible : public ABuildingActor
{
public:
    static UClass* StaticClass();

    bool ServerOnAttemptInteract(const FInteractionType& InteractType);
};
