#include "pch.h"
#include "FortInteractInterface.h"
#include "Building/BuildingGameplayActorConversationCompatible.h"
#include "Building/BuildingProp_ConversationCompatible.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

FInteractionType::FInteractionType(AFortPlayerPawn* InRequestingPawn, ETInteractionType InInteractionType)
    : InteractionType(ETInteractionType::IT_NoInteraction)
    , Data()
{
    static auto Constructor = Anchor::Function<void (*)(FInteractionType*, AFortPlayerPawn*, ETInteractionType)>(TEXT("FInteractionType::FInteractionType"))
        .String("AFortPlayerPawn::TryToAutoPickupWeapon")
        .Start()
        .Find("sbb rdx, rdx; and rdx, r*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("TryToAutoPickupWeapon: '%s'"))
        .Start()
        .Find("mov r8d, 0x1; lea rcx, [rbp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("TryToAutoPickupWeapon: '%s'"))
        .Start()
        .Find("sbb rdx, rdx; and rdx, r*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String("AFortPlayerPawn::TryToAutoPickupWeapon")
        .Start()
        .Find("sbb rdx, rdx; and rdx, r*; mov r8d, 0x1; lea rcx, [rbp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    Constructor(this, InRequestingPawn, InInteractionType);
}

AController* FInteractionType::GetController() const
{
    static auto GetController = Anchor::Function<AController* (*)(const FInteractionType*)>(TEXT("FInteractionType::GetController"))
        .At(DETOUR_ORIGINAL(ABuildingGameplayActorConversationCompatible, ServerOnAttemptInteract))
        .Find("cmp byte ptr [rbx], 0x1; jnz 0x*; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .At(DETOUR_ORIGINAL(ABuildingProp_ConversationCompatible, ServerOnAttemptInteract))
        .Find("cmp byte ptr [rbx], 0x1; jnz 0x*; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetController(this);
}

UClass* IFortInteractInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInteractInterface"));

    return Class;
}
