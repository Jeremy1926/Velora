#pragma once

#include "Components/ActorComponent.h"
#include "UObject/NameTypes.h"

class AActor;

class UFortControllerComponent_CraftingNetworkEvents : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerStartCrafting_Implementation(AActor* CraftingObject, const FName& CraftingFormulaName);

    void ServerCancelCrafting_Implementation(AActor* CraftingObject);

    void ServerClaimCraftingResults_Implementation(AActor* CraftingObject);

    void ServerEjectItems_Implementation(AActor* CraftingObject);
};
