#include "pch.h"
#include "Building/BuildingProp_ConversationCompatible.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "ConversationLibrary.h"
#include "FortInteractInterface.h"
#include "GameFramework/Controller.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"
#include "Detour.h"

UClass* ABuildingProp_ConversationCompatible::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingProp_ConversationCompatible"));

    return Class;
}

bool ABuildingProp_ConversationCompatible::ServerOnAttemptInteract(const FInteractionType& InteractType)
{
    DETOUR(ABuildingProp_ConversationCompatible, ServerOnAttemptInteract, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortInteractInterface:ServerOnAttemptInteract"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingProp_ConversationCompatible"), TEXT("/Script/FortniteGame.FortInteractInterface"))
        .Has("cmp byte ptr [rbx], 0x1; jnz 0x*; mov rcx, rbx; call 0x*; test rax, rax; jz 0x*; call 0x*; mov rbx, [rsp+0x*]; mov al, sil")
        .Optional()
        .Exec(TEXT("/Script/FortniteGame.FortInteractInterface:ServerOnAttemptInteract"))
        .Find("call [rax+0x*]")
        .InInterfaceVTable(TEXT("/Script/FortniteGame.Default__BuildingProp_ConversationCompatible"), TEXT("/Script/FortniteGame.FortInteractInterface")));

    static auto ConversationParticipantComponent = Anchor::Offset(TEXT("ABuildingProp_ConversationCompatible conversation participant component"))
        .At(DETOUR_ORIGINAL(ABuildingProp_ConversationCompatible, ServerOnAttemptInteract))
        .Find("mov eax, [rdi+0x*]")
        .Disp();
    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortInteractInterface::StaticClass()) - (uint8*)DefaultObject;

    const bool bResult = Original(InteractType);
    auto ParticipantComponent = ConversationParticipantComponent > 0 ? ((const TWeakObjectPtr<UFortNonPlayerConversationParticipantComponent>*)((const uint8*)this + ConversationParticipantComponent))->Get() : nullptr;

    if (!ParticipantComponent || !bResult || InteractType.InteractionType != ETInteractionType::IT_Simple)
        return bResult;

    auto Actor = (ABuildingProp_ConversationCompatible*)((uint8*)this - PointerOffset);

    auto PlayerController = Cast<AFortPlayerController>(InteractType.GetController());

    if (!PlayerController)
        return bResult;

    ParticipantComponent->InitializeServices();
    UConversationLibrary::StartConversation(ParticipantComponent->GetConversationEntryTag(), PlayerController, ParticipantComponent->GetInteractorParticipantTag(), Actor, ParticipantComponent->GetSelfParticipantTag());

    return bResult;
}
