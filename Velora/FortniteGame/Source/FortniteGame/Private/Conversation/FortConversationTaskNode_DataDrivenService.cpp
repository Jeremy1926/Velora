#include "pch.h"
#include "Conversation/FortConversationTaskNode_DataDrivenService.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "AI/FortAthenaAIBotController.h"
#include "Player/FortControllerEffect.h"
#include "Player/FortPlayerController.h"
#include "GameplayEffectTypes.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"

struct FDataDrivenServiceUsedStock
{
    TWeakObjectPtr<UConversationParticipantComponent> NPCComponent;
    const UFortConversationTaskNode_DataDrivenService* Service;
    int32 Count;
};

static TArray<FDataDrivenServiceUsedStock> UsedStock;

UClass* UFortConversationTaskNode_DataDrivenService::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_DataDrivenService"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_DataDrivenService::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto NPCComponent = UFortConversationContextLibrary::GetParticipantComponentNPC(Context);

    if (!PlayerController || !NPCComponent || CheckForFailedRequirements(Context))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const int32 StockCount = (int32)GetLimitedStockCount().GetValueAtLevel(0.0f);
    FDataDrivenServiceUsedStock* Stock = nullptr;

    for (int32 Index = UsedStock.Num() - 1; Index >= 0; Index--)
    {
        if (!UsedStock[Index].NPCComponent.Get())
        {
            UsedStock.RemoveAt(Index);
            continue;
        }

        if (UsedStock[Index].NPCComponent.Get() == NPCComponent && UsedStock[Index].Service == this)
            Stock = &UsedStock[Index];
    }

    if (StockCount > 0 && Stock && Stock->Count >= StockCount)
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const TOptional<int32> Price = FindServicePrice(GetPricingKey(), GetPricingColumn());

    if (!Price.IsSet() || !PayPrice(Context, Price))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    ApplyEffects(Context);

    if (Stock)
        Stock->Count++;
    else
        UsedStock.Add(FDataDrivenServiceUsedStock{ NPCComponent, this, 1 });

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_DataDrivenService::ApplyEffects(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto NPCComponent = Cast<UFortNPCConversationParticipantComponent>(UFortConversationContextLibrary::GetParticipantComponentNPC(Context));
    AController* NPCController = NPCComponent ? NPCComponent->GetBotControllerOwner() : nullptr;

    FEffectRequestContext RequestContext;
    RequestContext.GetInstigatingController() = NPCController;
    RequestContext.SetInstigatingActor(UFortConversationContextLibrary::GetParticipantActorNPC(Context));

    for (auto& Config : GetEffectRecipientConfigs())
    {
        AController* TargetController = Config.Recipient == EDataDrivenEffectRecipient::NPC ? NPCController : (AController*)PlayerController;

        if (!TargetController)
            continue;

        for (auto Effect : Config.Effects)
        {
            if (Effect)
                Effect->ApplyEffect(TargetController, RequestContext);
        }
    }
}

const FConversationParticipantRequirement* UFortConversationTaskNode_DataDrivenService::CheckForFailedRequirements(const FConversationContext& Context) const
{
    static auto CheckForFailedRequirements = Anchor::Function<const FConversationParticipantRequirement* (*)(const UFortConversationTaskNode_DataDrivenService*, const FConversationContext&)>(TEXT("UFortConversationTaskNode_DataDrivenService::CheckForFailedRequirements"))
        .String(TEXT("[%s]: Service has no cost set, returning FailedAndHidden"))
        .Start()
        .Find("mov rdx, rsi; mov rcx, rdi; call 0x*; test rax, rax")
        .Find("call 0x*")
        .Target()
        .Get();

    return CheckForFailedRequirements(this, Context);
}

const TArray<FEffectRecipientConfig>& UFortConversationTaskNode_DataDrivenService::GetEffectRecipientConfigs() const
{
    static auto EffectRecipientConfigs = StaticClass()->FindPropertyByName(FName(TEXT("EffectRecipientConfigs")));

    return *EffectRecipientConfigs->ContainerPtrToValuePtr<TArray<FEffectRecipientConfig>>(this);
}

FName UFortConversationTaskNode_DataDrivenService::GetPricingKey() const
{
    static auto PricingKey = StaticClass()->FindPropertyByName(FName(TEXT("PricingKey")));

    return *PricingKey->ContainerPtrToValuePtr<FName>(this);
}

int32 UFortConversationTaskNode_DataDrivenService::GetPricingColumn() const
{
    static auto PricingColumn = StaticClass()->FindPropertyByName(FName(TEXT("PricingColumn")));

    return *PricingColumn->ContainerPtrToValuePtr<int32>(this);
}

const FScalableFloat& UFortConversationTaskNode_DataDrivenService::GetLimitedStockCount() const
{
    static auto LimitedStockCount = StaticClass()->FindPropertyByName(FName(TEXT("LimitedStockCount")));

    return *LimitedStockCount->ContainerPtrToValuePtr<FScalableFloat>(this);
}
