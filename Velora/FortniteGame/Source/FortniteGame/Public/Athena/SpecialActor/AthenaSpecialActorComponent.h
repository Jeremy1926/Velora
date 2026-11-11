#pragma once

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/NameTypes.h"

class UAthenaSpecialActorComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void Activate(bool bReset);

    void Deactivate();

    void RegisterSpecialActor();

    void UnregisterSpecialActor();

    FGameplayTag GetSpecialActorTag() const;

    FName GetSpecialActorID() const;

    void SetSpecialActorID(FName InSpecialActorID);
};
