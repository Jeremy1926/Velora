#include "pch.h"
#include "Conversation/FortConversationTaskNode_Service.h"
#include "FortKismetLibrary.h"
#include "Items/FortInventory.h"
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

int32 UFortConversationTaskNode_Service::GetCurrencyCount(AFortPlayerController* PlayerController) const
{
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;
    auto ResourceCurrency = GetResourceCurrency();
    int32 Count = 0;

    if (!WorldInventory || !ResourceCurrency)
        return Count;

    for (auto& ItemEntry : WorldInventory->GetInventory().GetReplicatedEntries())
    {
        if (ItemEntry.GetItemDefinition() == ResourceCurrency)
            Count += ItemEntry.GetCount();
    }

    return Count;
}

bool UFortConversationTaskNode_Service::CanPlayerAffordPrice(AFortPlayerController* PlayerController, int32 Price) const
{
    return Price <= 0 || GetCurrencyCount(PlayerController) >= Price;
}

bool UFortConversationTaskNode_Service::PayPrice(AFortPlayerController* PlayerController, int32 Price) const
{
    if (!CanPlayerAffordPrice(PlayerController, Price))
        return false;

    if (Price > 0)
        UFortKismetLibrary::K2_RemoveItemFromPlayer(PlayerController, GetResourceCurrency(), Price, true);

    return true;
}
