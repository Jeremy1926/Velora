#include "pch.h"
#include "Building/BuildingGameplayActorConversationCompatible.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "ConversationLibrary.h"
#include "FortInteractInterface.h"
#include "GameFramework/Controller.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* ABuildingGameplayActorConversationCompatible::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingGameplayActorConversationCompatible"));

    return Class;
}

bool ABuildingGameplayActorConversationCompatible::ServerOnAttemptInteract(const FInteractionType& InteractType)
{
    DETOUR(ABuildingGameplayActorConversationCompatible, ServerOnAttemptInteract, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortInteractInterface:ServerOnAttemptInteract"))
        .Find("call [rax+0x*]")
        .InInterfaceVTables(StaticClass(), IFortInteractInterface::StaticClass(), "mov [rsp+0x8], rbx; mov [rsp+0x10], rsi; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rsi, rcx; call 0x*; mov dil, al; test al, al; jz 0x*; cmp byte ptr [rbx], 0x1; jnz 0x*; mov rcx, rbx; call 0x*; mov rbx, rax; test rax, rax; jz 0x*; call 0x*; mov rcx, [rbx+0x10]; mov rdx, rax; call 0x*; test al, al; jz 0x*; lea rcx, [rsi-0x*]; call 0x*; test rax, rax; jz 0x*; mov r8, [rax]; mov rdx, rbx; mov rcx, rax; call [r8+0x*]; mov rbx, [rsp+0x*]"));

    static auto CanStartConversationWith = Anchor::VirtualFunction<bool (*)(const UFortNonPlayerConversationParticipantComponent*, const AFortPlayerController*)>(TEXT("UFortNonPlayerConversationParticipantComponent::CanStartConversationWith"))
        .At(DETOUR_ORIGINAL(ABuildingGameplayActorConversationCompatible, ServerOnAttemptInteract))
        .Find("mov rcx, rax; call [r8+0x*]")
        .Find("call [r8+0x*]")
        .Get();
    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortInteractInterface::StaticClass()) - (uint8*)DefaultObject;

    const bool bResult = Original(InteractType);

    if (!bResult || InteractType.InteractionType != ETInteractionType::IT_Simple)
        return bResult;

    auto Actor = (ABuildingGameplayActorConversationCompatible*)((uint8*)this - PointerOffset);
    auto PlayerController = Cast<AFortPlayerController>(InteractType.GetController());
    auto ParticipantComponent = PlayerController ? Actor->FindComponentByClass<UFortNonPlayerConversationParticipantComponent>() : nullptr;

    if (!ParticipantComponent || !CanStartConversationWith(ParticipantComponent, PlayerController))
        return bResult;

    ParticipantComponent->InitializeServices();
    UConversationLibrary::StartConversation(ParticipantComponent->GetConversationEntryTag(), PlayerController, ParticipantComponent->GetInteractorParticipantTag(), Actor, ParticipantComponent->GetSelfParticipantTag());

    return bResult;
}
