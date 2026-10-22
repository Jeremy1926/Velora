#include "pch.h"
#include "Conversation/FortConversationTaskNode_GiveGift.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortConversationGiftTypeDefinitions.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "Items/FortInventory.h"
#include "Items/FortLootPackage.h"
#include "Items/FortLootTier.h"
#include "Items/FortWorldItemDefinition.h"
#include "Player/FortControllerEffect.h"
#include "Player/FortControllerRequirement.h"
#include "Player/FortPlayerController.h"
#include "AI/FortAthenaAIBotController.h"
#include "FortKismetLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include <utility>
#include <vector>

UClass* UFortConversationTaskNode_GiveGift::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_GiveGift"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_GiveGift::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    static std::vector<std::pair<TWeakObjectPtr<UFortNPCConversationParticipantComponent>, TWeakObjectPtr<AFortPlayerController>>> GiftedPlayers;

    auto NPCComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);

    if (!NPCComponent || !PlayerController)
        return FConversationTaskResult::AdvanceConversation();

    for (size_t Index = GiftedPlayers.size(); Index > 0; Index--)
    {
        auto& Entry = GiftedPlayers[Index - 1];

        if (!Entry.first.Get() || !Entry.second.Get())
        {
            GiftedPlayers.erase(GiftedPlayers.begin() + (Index - 1));
            continue;
        }

        if (Entry.first.Get() == NPCComponent && Entry.second.Get() == PlayerController)
            return FConversationTaskResult::AdvanceConversation();
    }

    GiftedPlayers.push_back({ NPCComponent, PlayerController });
    GiveGift(NPCComponent, PlayerController);

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_GiveGift::GiveGift(UFortNPCConversationParticipantComponent* NPCComponent, AFortPlayerController* PlayerController) const
{
    static auto NPCConversationGiftRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCConversationGiftRow"));
    static auto GiftTag = NPCConversationGiftRow ? NPCConversationGiftRow->FindPropertyByName(FName(TEXT("GiftTag"))) : nullptr;
    static auto Weight = NPCConversationGiftRow ? NPCConversationGiftRow->FindPropertyByName(FName(TEXT("Weight"))) : nullptr;
    static auto NPCGiveItemClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_NPCGiveItem"));

    auto GiftTypeDefinitions = NPCComponent->GetGiftTypeDefinitions();

    if (!GiftTypeDefinitions || !GiftTag || !Weight)
        return;

    std::vector<std::pair<float, const FConversationGiftTypeDefinition*>> Candidates;
    float TotalWeight = 0.0f;

    for (auto GiftRow : NPCComponent->GetSupportedGiftRows())
    {
        const float GiftWeight = *Weight->ContainerPtrToValuePtr<float>(GiftRow);
        const FGameplayTag& RowGiftTag = *GiftTag->ContainerPtrToValuePtr<FGameplayTag>(GiftRow);

        if (GiftWeight <= 0.0f)
            continue;

        for (auto& DefinitionPair : GiftTypeDefinitions->GetGiftDefinitionsByGiftTag())
        {
            if (!(DefinitionPair.Key == RowGiftTag))
                continue;

            const auto& Definition = DefinitionPair.Value;
            bool bAllMet = true;
            bool bAnyMet = false;

            for (auto Requirement : Definition.Requirements)
            {
                const bool bMet = Requirement && Requirement->IsRequirementMet(PlayerController);

                bAllMet &= bMet;
                bAnyMet |= bMet;
            }

            if (Definition.Requirements.Num() && !(Definition.RequirementMatchPolicy ? bAnyMet : bAllMet))
                break;

            Candidates.push_back({ GiftWeight, &Definition });
            TotalWeight += GiftWeight;
            break;
        }
    }

    if (Candidates.empty())
        return;

    float Roll = FMath::FRand() * TotalWeight;
    auto Picked = Candidates.back().second;

    for (auto& Candidate : Candidates)
    {
        Roll -= Candidate.first;

        if (Roll <= 0.0f)
        {
            Picked = Candidate.second;
            break;
        }
    }

    for (auto Effect : Picked->Effects)
    {
        if (!Effect)
            continue;

        if (NPCGiveItemClass && Effect->IsA(NPCGiveItemClass))
        {
            GiveGiftItems(NPCComponent, PlayerController);
            continue;
        }

        if (auto ApplyGameplayEffect = Cast<UFortControllerEffect_ApplyGameplayEffect>(Effect))
        {
            ApplyGameplayEffect->ApplyGameplayEffect(PlayerController);
            continue;
        }

        FEffectRequestContext RequestContext;
        RequestContext.InstigatingController = NPCComponent->GetBotControllerOwner();

        Effect->ApplyEffect(PlayerController, RequestContext);
    }
}

void UFortConversationTaskNode_GiveGift::GiveGiftItems(UFortNPCConversationParticipantComponent* NPCComponent, AFortPlayerController* PlayerController) const
{
    static auto NPCGiftItemInventoryRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCGiftItemInventoryRow"));
    static auto LootTier = NPCGiftItemInventoryRow ? NPCGiftItemInventoryRow->FindPropertyByName(FName(TEXT("LootTier"))) : nullptr;
    static auto LootLevel = NPCGiftItemInventoryRow ? NPCGiftItemInventoryRow->FindPropertyByName(FName(TEXT("LootLevel"))) : nullptr;
    static auto OverrideNumDrops = NPCGiftItemInventoryRow ? NPCGiftItemInventoryRow->FindPropertyByName(FName(TEXT("OverrideNumDrops"))) : nullptr;
    static auto LootTags = NPCGiftItemInventoryRow ? NPCGiftItemInventoryRow->FindPropertyByName(FName(TEXT("LootTags"))) : nullptr;

    auto GiftItemRows = NPCComponent->GetSupportedGiftItemRows();

    if (!GiftItemRows.Num() || !LootTier || !LootLevel || !OverrideNumDrops || !LootTags)
        return;

    const uint8* GiftItemRow = GiftItemRows[FMath::RandHelper(GiftItemRows.Num())];
    const int32 WorldLevel = *LootLevel->ContainerPtrToValuePtr<int32>(GiftItemRow);
    const int32 NumDropsOverride = *OverrideNumDrops->ContainerPtrToValuePtr<int32>(GiftItemRow);
    const FGameplayTagContainer& GameplayTags = *LootTags->ContainerPtrToValuePtr<FGameplayTagContainer>(GiftItemRow);
    FName LootTierKey(NAME_None);
    int32 ChosenLootTier = -1;

    if (!UFortLootTier::ChooseUnquotaedLootTierKey(LootTierKey, ChosenLootTier, *LootTier->ContainerPtrToValuePtr<FName>(GiftItemRow), WorldLevel, GameplayTags))
        return;

    TArray<FFortItemEntry> LootToDrop;

    if (!UFortLootPackage::PickLootDrops(LootToDrop, WorldLevel, LootTierKey, GameplayTags, false, false))
        return;

    const int32 NumDrops = NumDropsOverride > 0 ? FMath::Min(NumDropsOverride, LootToDrop.Num()) : LootToDrop.Num();

    for (int32 Index = 0; Index < NumDrops; Index++)
    {
        if (auto ItemDefinition = Cast<UFortWorldItemDefinition>(LootToDrop[Index].GetItemDefinition()))
            UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, ItemDefinition, FMath::Max(LootToDrop[Index].GetCount(), 1));
    }
}
