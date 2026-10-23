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
        .InInterfaceVTables(StaticClass(), IFortInteractInterface::StaticClass(), "mov [rsp+0x8], rbx; mov [rsp+0x10], rsi; push rdi; sub rsp, 0x20; mov rbx, rdx; mov rdi, rcx; call 0x*; mov r9d, [rdi+0x*]; mov sil, al; test r9d, r9d; jz 0x*; mov eax, [rdi+0x*]; test eax, eax; js 0x*; cmp eax, [0x*]; jnl 0x*; cdq; mov ecx, 0x10000; idiv ecx; movsxd rcx, eax; mov rax, [0x*]; mov r8, [rax+rcx*8]; movsxd rax, edx; lea rcx, [rax+rax*2]; lea rax, [r8+rcx*8]; test rax, rax; jz 0x*; cmp [rax+0x10], r9d; jnz 0x*; test dword ptr [rax+0x8], 0x10000000; jnz 0x*; test dword ptr [rax+0x8], 0x20000000; jnz 0x*; cmp qword ptr [rax], 0x0; jz 0x*; test sil, sil; jz 0x*; cmp byte ptr [rbx], 0x1; jnz 0x*; mov rcx, rbx; call 0x*; test rax, rax; jz 0x*; call 0x*; mov rbx, [rsp+0x*]; mov al, sil"));

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

    if (auto PlayerController = Cast<AFortPlayerController>(InteractType.GetController()))
        UConversationLibrary::StartConversation(ParticipantComponent->GetConversationEntryTag(), PlayerController, ParticipantComponent->GetInteractorParticipantTag(), Actor, ParticipantComponent->GetSelfParticipantTag());

    return bResult;
}
