#pragma once

#include "Building/BuildingActor.h"

struct FInteractionType;

class ABuildingGameplayActorConversationCompatible : public ABuildingActor
{
public:
    static UClass* StaticClass();

    bool ServerOnAttemptInteract(const FInteractionType& InteractType);
};
