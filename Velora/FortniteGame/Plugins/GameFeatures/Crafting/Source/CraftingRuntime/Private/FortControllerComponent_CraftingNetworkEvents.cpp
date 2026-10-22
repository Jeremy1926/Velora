#include "pch.h"
#include "FortControllerComponent_CraftingNetworkEvents.h"
#include "CraftingObjectComponent.h"
#include "FortGameStateComponent_Crafting.h"
#include "GameFramework/Actor.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortControllerComponent_CraftingNetworkEvents::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents"));

    return Class;
}

void UFortControllerComponent_CraftingNetworkEvents::ServerStartCrafting_Implementation(AActor* CraftingObject, const FName& CraftingFormulaName)
{
    DETOUR(UFortControllerComponent_CraftingNetworkEvents, ServerStartCrafting_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents:ServerStartCrafting"))
        .Find("call [rax+0x*]", 2)
        .Optional()
        .InVTable(TEXT("/Script/CraftingRuntime.Default__FortControllerComponent_CraftingNetworkEvents")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto CraftingObjectComponent = CraftingObject ? CraftingObject->FindComponentByClass<UCraftingObjectComponent>() : nullptr;

    if (!PlayerController || !PlayerController->HasAuthority() || !CraftingObjectComponent)
        return;

    if (auto GameStateComponent = UFortGameStateComponent_Crafting::Get(PlayerController))
        GameStateComponent->UpdateCraftingResultsList();

    CraftingObjectComponent->StartCrafting(PlayerController, CraftingFormulaName);
}

void UFortControllerComponent_CraftingNetworkEvents::ServerPickupItemAndStartCrafting_Implementation(AActor* CraftingObject, AFortPickup* Pickup, const FName& CraftingFormulaName)
{
    DETOUR(UFortControllerComponent_CraftingNetworkEvents, ServerPickupItemAndStartCrafting_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents:ServerPickupItemAndStartCrafting"))
        .Find("call [rax+0x*]", 2)
        .Optional()
        .InVTable(TEXT("/Script/CraftingRuntime.Default__FortControllerComponent_CraftingNetworkEvents")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto CraftingObjectComponent = CraftingObject ? CraftingObject->FindComponentByClass<UCraftingObjectComponent>() : nullptr;

    if (!PlayerController || !PlayerController->HasAuthority() || !CraftingObjectComponent || !Pickup)
        return;

    if (auto GameStateComponent = UFortGameStateComponent_Crafting::Get(PlayerController))
        GameStateComponent->UpdateCraftingResultsList();

    CraftingObjectComponent->PickupItemAndStartCrafting(PlayerController, Pickup, CraftingFormulaName);
}

void UFortControllerComponent_CraftingNetworkEvents::ServerCancelCrafting_Implementation(AActor* CraftingObject)
{
    DETOUR(UFortControllerComponent_CraftingNetworkEvents, ServerCancelCrafting_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents:ServerCancelCrafting"))
        .Find("call [rax+0x*]", 2)
        .Optional()
        .InVTable(TEXT("/Script/CraftingRuntime.Default__FortControllerComponent_CraftingNetworkEvents")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto CraftingObjectComponent = CraftingObject ? CraftingObject->FindComponentByClass<UCraftingObjectComponent>() : nullptr;

    if (PlayerController && PlayerController->HasAuthority() && CraftingObjectComponent)
        CraftingObjectComponent->CancelCrafting();
}

void UFortControllerComponent_CraftingNetworkEvents::ServerClaimCraftingResults_Implementation(AActor* CraftingObject)
{
    DETOUR(UFortControllerComponent_CraftingNetworkEvents, ServerClaimCraftingResults_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents:ServerClaimCraftingResults"))
        .Find("call [rax+0x*]", 2)
        .Optional()
        .InVTable(TEXT("/Script/CraftingRuntime.Default__FortControllerComponent_CraftingNetworkEvents")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto CraftingObjectComponent = CraftingObject ? CraftingObject->FindComponentByClass<UCraftingObjectComponent>() : nullptr;

    if (PlayerController && PlayerController->HasAuthority() && CraftingObjectComponent)
        CraftingObjectComponent->ClaimCraftingResults();
}

void UFortControllerComponent_CraftingNetworkEvents::ServerEjectItems_Implementation(AActor* CraftingObject)
{
    DETOUR(UFortControllerComponent_CraftingNetworkEvents, ServerEjectItems_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/CraftingRuntime.FortControllerComponent_CraftingNetworkEvents:ServerEjectItems"))
        .Find("call [rax+0x*]", 2)
        .Optional()
        .InVTable(TEXT("/Script/CraftingRuntime.Default__FortControllerComponent_CraftingNetworkEvents")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto CraftingObjectComponent = CraftingObject ? CraftingObject->FindComponentByClass<UCraftingObjectComponent>() : nullptr;

    if (PlayerController && PlayerController->HasAuthority() && CraftingObjectComponent)
        CraftingObjectComponent->EjectItems();
}
