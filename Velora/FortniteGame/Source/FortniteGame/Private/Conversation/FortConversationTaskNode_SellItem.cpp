#include "pch.h"
#include "Conversation/FortConversationTaskNode_SellItem.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "Items/FortInventory.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortWorldItemDefinition.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/FortPlayerController.h"
#include "FortGameData.h"
#include "FortKismetLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include <list>

UClass* UFortConversationTaskNode_SellItem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_SellItem"));

    return Class;
}

int32 UFortConversationTaskNode_SellItem::GetSellSlot() const
{
    static auto SellSlot = StaticClass()->FindPropertyByName(FName(TEXT("SellSlot")));

    return *SellSlot->ContainerPtrToValuePtr<int32>(this);
}

const FText& UFortConversationTaskNode_SellItem::GetBuyTextFormat() const
{
    static auto BuyTextFormat = StaticClass()->FindPropertyByName(FName(TEXT("BuyTextFormat")));

    return *BuyTextFormat->ContainerPtrToValuePtr<FText>(this);
}

const FText& UFortConversationTaskNode_SellItem::GetSoldOutTextFormat() const
{
    static auto SoldOutTextFormat = StaticClass()->FindPropertyByName(FName(TEXT("SoldOutTextFormat")));

    return *SoldOutTextFormat->ContainerPtrToValuePtr<FText>(this);
}

FConversationSellTaskMemory* UFortConversationTaskNode_SellItem::GetSellingData(const FConversationContext& Context) const
{
    struct FSellingDataEntry
    {
        TWeakObjectPtr<UFortNPCConversationParticipantComponent> NPCComponent;
        int32 SellSlot;
        FConversationSellTaskMemory SellingData;
    };

    static std::list<FSellingDataEntry> SellingDatas;
    static auto NPCSaleInventoryRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCSaleInventoryRow"));
    static auto LootTier = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("LootTier")));
    static auto LootLevel = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("LootLevel")));
    static auto LootTags = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("LootTags")));

    auto NPCComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));

    if (!NPCComponent)
        return nullptr;

    const int32 SellSlot = GetSellSlot();

    for (auto& Entry : SellingDatas)
    {
        if (Entry.SellSlot == SellSlot && Entry.NPCComponent.Get() == NPCComponent)
            return &Entry.SellingData;
    }

    SellingDatas.remove_if([](const FSellingDataEntry& Entry)
    {
        return !Entry.NPCComponent.Get();
    });

    auto& Entry = SellingDatas.emplace_back();
    Entry.NPCComponent = NPCComponent;
    Entry.SellSlot = SellSlot;

    auto SaleRows = NPCComponent->GetSupportedSaleRows();

    if (SellSlot < 1 || SellSlot > SaleRows.Num())
        return &Entry.SellingData;

    const uint8* SaleRow = SaleRows[SellSlot - 1];
    const int32 WorldLevel = *LootLevel->ContainerPtrToValuePtr<int32>(SaleRow);
    const FGameplayTagContainer& GameplayTags = *LootTags->ContainerPtrToValuePtr<FGameplayTagContainer>(SaleRow);
    FName LootTierKey(NAME_None);
    int32 ChosenLootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, ChosenLootTier, *LootTier->ContainerPtrToValuePtr<FName>(SaleRow), WorldLevel, GameplayTags))
        return &Entry.SellingData;

    auto LootTierData = UFortGameData::Get()->FindLootTierDataRow(LootTierKey, TEXT("UFortConversationTaskNode_SellItem::GetSellingData"));

    if (!LootTierData || LootTierData->GetLootPackage() == NAME_None)
        return &Entry.SellingData;

    TArray<FFortItemEntry> LootToDrop;
    UFortLootPackage::PickLootDropsFromLootPackage(LootToDrop, LootTierData->GetLootPackage(), WorldLevel, 0, GameplayTags, false);

    for (auto& LootEntry : LootToDrop)
    {
        if (!LootEntry.GetItemDefinition())
            continue;

        Entry.SellingData.LootForSaleDefinition = LootEntry.GetItemDefinition();
        Entry.SellingData.LootForSaleStackSize = LootEntry.GetCount();
        break;
    }

    const float NumLootPackageDrops = LootTierData->GetNumLootPackageDrops();
    int32 NumDrops = NumLootPackageDrops > 0.0f ? 1 : 0;

    if (NumLootPackageDrops >= 1.0f)
    {
        NumDrops = FMath::FloorToInt(NumLootPackageDrops);

        const float Remainder = NumLootPackageDrops - NumDrops;

        if (Remainder > 1.e-5f)
            NumDrops += (Remainder >= FMath::FRand()) ? 1 : 0;
    }

    Entry.SellingData.OriginalLootCountForSale = NumDrops;
    Entry.SellingData.LootRemainingForSale = NumDrops;

    return &Entry.SellingData;
}

TOptional<int32> UFortConversationTaskNode_SellItem::GetLootCost(const FConversationSellTaskMemory& SellingData) const
{
    auto ItemDefinition = SellingData.LootForSaleDefinition.Get();

    if (!ItemDefinition)
        return TOptional<int32>();

    TOptional<int32> Price = FindServicePrice(FName(*UKismetSystemLibrary::GetPathName(ItemDefinition)), (int32)ItemDefinition->GetRarity());

    if (Price.IsSet())
        Price.Value *= FMath::Max(SellingData.LootForSaleStackSize, 1);

    return Price;
}

FConversationTaskResult UFortConversationTaskNode_SellItem::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto SellingData = GetSellingData(Context);
    auto ItemDefinition = SellingData ? Cast<UFortWorldItemDefinition>(SellingData->LootForSaleDefinition.Get()) : nullptr;

    if (!PlayerController || !ItemDefinition || SellingData->LootRemainingForSale <= 0)
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const TOptional<int32> LootCost = GetLootCost(*SellingData);

    if (!PayPrice(Context, LootCost))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, ItemDefinition, FMath::Max(SellingData->LootForSaleStackSize, 1));
    SellingData->LootRemainingForSale--;

    return FConversationTaskResult::AdvanceConversation();
}

EConversationRequirementResult UFortConversationTaskNode_SellItem::IsRequirementSatisfied_Implementation(const FConversationContext& Context) const
{
    auto SellingData = GetSellingData(Context);

    if (!SellingData || !SellingData->LootForSaleDefinition.Get())
        return EConversationRequirementResult::FailedAndHidden;

    const TOptional<int32> LootCost = GetLootCost(*SellingData);

    if (SellingData->LootRemainingForSale <= 0 || !CanPlayerAffordPrice(Context, LootCost))
        return EConversationRequirementResult::FailedButVisible;

    return EConversationRequirementResult::Passed;
}

void UFortConversationTaskNode_SellItem::GatherDynamicChoices(FConversationBranchPointBuilder& BranchBuilder, const FConversationContext& Context) const
{
    auto SellingData = GetSellingData(Context);
    auto ItemDefinition = SellingData ? SellingData->LootForSaleDefinition.Get() : nullptr;

    if (!ItemDefinition)
        return;

    const EConversationRequirementResult RequirementResult = CheckRequirements(Context);

    if (RequirementResult >= EConversationRequirementResult::FailedAndHidden)
        return;

    const TOptional<int32> LootCost = GetLootCost(*SellingData);

    FClientConversationOptionEntry Choice;
    Choice.ChoiceReference.NodeReference = GetNodeGuid();
    Choice.ChoiceText = SellingData->LootRemainingForSale > 0 ? FText::Format(FTextFormat(GetBuyTextFormat()), FFormatArgumentValue(ItemDefinition->GetDisplayName())) : GetSoldOutTextFormat();
    Choice.SetChoiceAvailable(RequirementResult == EConversationRequirementResult::Passed);
    Choice.ChoiceReference.NodeParameters.Add({ FString(TEXT("item")), UKismetSystemLibrary::GetPathName(ItemDefinition) });
    Choice.ChoiceReference.NodeParameters.Add({ FString(TEXT("cost")), FString::FromInt(LootCost.IsSet() ? LootCost.GetValue() : 0) });
    Choice.ChoiceReference.NodeParameters.Add({ FString(TEXT("stacksize")), FString::FromInt(SellingData->LootForSaleStackSize) });
    Choice.ChoiceReference.NodeParameters.Add({ FString(TEXT("remaining")), FString::FromInt(SellingData->LootRemainingForSale) });

    GatherStaticExtraData(Context, Choice.ExtraData);
    BranchBuilder.AddChoice(Context, Choice);
}
