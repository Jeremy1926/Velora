#include "pch.h"
#include "FortPlayerConversationComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortPlayerConversationComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteConversationRuntime.FortPlayerConversationComponent"));

    return Class;
}

void UFortPlayerConversationComponent::RequestServerAbortConversation_Implementation()
{
    DETOUR(UFortPlayerConversationComponent, RequestServerAbortConversation_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteConversationRuntime.FortPlayerConversationComponent:RequestServerAbortConversation"))
        .Find("jmp [*+0x*]")
        .IsVirtual(TEXT("/Script/FortniteConversationRuntime.Default__FortPlayerConversationComponent"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteConversationRuntime.Default__FortPlayerConversationComponent")));

    AActor* Owner = GetOwner();

    if (Owner && Owner->HasAuthority())
        ServerAbortAllConversations();
}
