#include "pch.h"
#include "Conversation/FortConversationTaskNode_FindActorByClass.h"
#include "GameplayEffectTypes.h"
#include "HAL/UnrealMemory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_FindActorByClass::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_FindActorByClass"));

    return Class;
}

TArray<AActor*> UFortConversationTaskNode_FindActorByClass::GetFoundActors(const FConversationContext& Context, FFindActorByClassData& OutFilterData) const
{
    static auto GetFoundActors = Anchor::Function<TArray<AActor*>* (*)(const UFortConversationTaskNode_FindActorByClass*, TArray<AActor*>*, const FConversationContext&, FFindActorByClassData&)>(TEXT("UFortConversationTaskNode_FindActorByClass::GetFoundActors"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_FindActorByClass"), 0)
        .Find("lea r9, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    TArray<AActor*> FoundActors;
    GetFoundActors(this, &FoundActors, Context, OutFilterData);

    return FoundActors;
}

void UFortConversationTaskNode_FindActorByClass::GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const
{
    UFortConversationTaskNode::GatherStaticExtraData_Implementation(Context, InOutExtraData);

    auto FilterData = (FFindActorByClassData*)FMemory::Realloc(nullptr, FFindActorByClassData::GetTypeSize(), 8);
    FFindActorByClassData::StaticStruct()->InitializeStruct(FilterData);

    if (GetFoundActors(Context, *FilterData).Num() > 0)
        InOutExtraData.Add({ FString(TEXT("cost")), FString::FromInt((int32)FilterData->GetPrice().GetValueAtLevel(FilterData->GetRarity().GetValueAtLevel(0.0f))) });

    FFindActorByClassData::StaticStruct()->DestroyStruct(FilterData);
    FMemory::Free(FilterData);
}

UScriptStruct* FFindActorByClassData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FindActorByClassData"));

    return Struct;
}

int32 FFindActorByClassData::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

const FScalableFloat& FFindActorByClassData::GetPrice() const
{
    static auto Price = StaticStruct()->FindPropertyByName(FName(TEXT("Price")));

    return *Price->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat& FFindActorByClassData::GetRarity() const
{
    static auto Rarity = StaticStruct()->FindPropertyByName(FName(TEXT("Rarity")));

    return *Rarity->ContainerPtrToValuePtr<FScalableFloat>(this);
}
