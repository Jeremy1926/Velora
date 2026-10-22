#pragma once

#include "Components/ActorComponent.h"
#include "UObject/NameTypes.h"

class AActor;
class AFortPickup;

class UFortControllerComponent_CraftingNetworkEvents : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerStartCrafting_Implementation(AActor* CraftingObject, const FName& CraftingFormulaName);

    void ServerPickupItemAndStartCrafting_Implementation(AActor* CraftingObject, AFortPickup* Pickup, const FName& CraftingFormulaName);

    void ServerCancelCrafting_Implementation(AActor* CraftingObject);

    void ServerClaimCraftingResults_Implementation(AActor* CraftingObject);

    void ServerEjectItems_Implementation(AActor* CraftingObject);
};
