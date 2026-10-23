#include "pch.h"
#include "FortInteractInterface.h"
#include "Building/BuildingGameplayActorConversationCompatible.h"
#include "Building/BuildingProp_ConversationCompatible.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

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
