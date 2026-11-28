#pragma once

#include "Components/ActorComponent.h"
#include "UObject/NameTypes.h"

class AActor;
class AFortPickup;
struct FCraftingMultiKey;

class UFortControllerComponent_CraftingNetworkEvents : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ServerStartCrafting_Implementation(AActor* CraftingObject, const FName& CraftingFormulaName, int32 NumberToCraft, FCraftingMultiKey Key, float OverriddenObjectStateLength);

    void ServerPickupItemAndStartCrafting_Implementation(AActor* CraftingObject, AFortPickup* Pickup, const FName& CraftingFormulaName);

    void ServerCancelCrafting_Implementation(AActor* CraftingObject, FCraftingMultiKey Key);

    void ServerClaimCraftingResults_Implementation(AActor* CraftingObject, FCraftingMultiKey Key);

    void ServerEjectItems_Implementation(AActor* CraftingObject);
};
