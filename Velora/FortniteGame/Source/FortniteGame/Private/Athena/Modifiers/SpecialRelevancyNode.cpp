#include "pch.h"
#include "Athena/Modifiers/SpecialRelevancyNode.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortReplicationGraphNode_SpecialRelevancy::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortReplicationGraphNode_SpecialRelevancy"));

    return Class;
}

TMulticastDelegate<void, bool>& UFortReplicationGraphNode_SpecialRelevancy::GetOnSpecialRelevancyEnabled()
{
    static auto OnSpecialRelevancyEnabled = Anchor::Offset(TEXT("UFortReplicationGraphNode_SpecialRelevancy::OnSpecialRelevancyEnabled"))
        .UseSlot(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_SpecialRelevancy"), 0)
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbx+0x*]; call 0x*; mov rcx, rbx; add rsp, 0x20; pop rbx; jmp 0x*")
        .Disp();

    return *(TMulticastDelegate<void, bool>*)((uint8*)this + OnSpecialRelevancyEnabled);
}

UClass* AFortAthenaMutator_SpecialRelevancy::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMutator_SpecialRelevancy"));

    return Class;
}

TSubclassOf<UFortReplicationGraphNode_SpecialRelevancy> AFortAthenaMutator_SpecialRelevancy::GetRootGridNodeClass() const
{
    static auto RootGridNodeClass = StaticClass()->FindPropertyByName(FName(TEXT("RootGridNodeClass")));

    return *RootGridNodeClass->ContainerPtrToValuePtr<TSubclassOf<UFortReplicationGraphNode_SpecialRelevancy>>(this);
}
