#include "pch.h"
#include "Conversation/FortConversationTaskNode_Service.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "FortKismetLibrary.h"
#include "Items/FortWorldItemDefinition.h"
#include "Player/FortPlayerController.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortConversationTaskNode_Service::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_Service"));

    return Class;
}

UFortWorldItemDefinition* UFortConversationTaskNode_Service::GetResourceCurrency() const
{
    static auto ResourceCurrency = StaticClass()->FindPropertyByName(FName(TEXT("ResourceCurrency")));

    return *ResourceCurrency->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

const TArray<UCurveTable*>& UFortConversationTaskNode_Service::GetPricingTables() const
{
    static auto PricingTables = StaticClass()->FindPropertyByName(FName(TEXT("PricingTables")));

    return *PricingTables->ContainerPtrToValuePtr<TArray<UCurveTable*>>(this);
}

TOptional<int32> UFortConversationTaskNode_Service::FindServicePrice(FName PricingRowKey, int32 PricingColumn) const
{
    static auto FindServicePrice = Anchor::Function<TOptional<int32>* (*)(const UFortConversationTaskNode_Service*, TOptional<int32>*, FName, int32)>(TEXT("UFortConversationTaskNode_Service::FindServicePrice"))
        .String(TEXT("UFortConversationTaskNode_Service::FindServicePrice"))
        .Start()
        .Get();

    TOptional<int32> Price;
    FindServicePrice(this, &Price, PricingRowKey, PricingColumn);

    return Price;
}

bool UFortConversationTaskNode_Service::CanPlayerAffordPrice(const FConversationContext& Context, const TOptional<int32>& Price) const
{
    static auto CanPlayerAffordPrice = Anchor::Function<bool (*)(const UFortConversationTaskNode_Service*, const FConversationContext&, TOptional<int32>*)>(TEXT("UFortConversationTaskNode_Service::CanPlayerAffordPrice"))
        .Exec(TEXT("/Script/CommonDialogueRuntime.ConversationTaskNode:IsRequirementSatisfied"))
        .Find("call [*+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortConversationTaskNode_UpgradeItem"), 0)
        .Call(3)
        .Get();

    TOptional<int32> InPrice = Price;

    return CanPlayerAffordPrice(this, Context, &InPrice);
}

bool UFortConversationTaskNode_Service::PayPrice(const FConversationContext& Context, const TOptional<int32>& Price) const
{
    if (!CanPlayerAffordPrice(Context, Price))
        return false;

    if (Price.GetValue() > 0)
        UFortKismetLibrary::K2_RemoveItemFromPlayer(UFortConversationContextLibrary::GetParticipantPlayerController(Context), GetResourceCurrency(), Price.GetValue(), true);

    return true;
}
