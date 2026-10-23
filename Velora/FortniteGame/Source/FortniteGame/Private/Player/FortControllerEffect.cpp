#include "pch.h"
#include "Player/FortControllerEffect.h"
#include "Conversation/FortConversationTaskNode_GiveGift.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffect.h"
#include "Pawns/FortPawn.h"
#include "Player/FortPlayerController.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"

FEffectRequestContext::FEffectRequestContext()
{
    FMemory::Memzero(Data, sizeof(Data));

    StaticStruct()->InitializeStruct(this);
}

FEffectRequestContext::~FEffectRequestContext()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FEffectRequestContext::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EffectRequestContext"));

    return Struct;
}

AController*& FEffectRequestContext::GetInstigatingController()
{
    static auto InstigatingController = StaticStruct()->FindPropertyByName(FName(TEXT("InstigatingController")));

    return *InstigatingController->ContainerPtrToValuePtr<AController*>(this);
}

void FEffectRequestContext::SetInstigatingActor(AActor* InstigatingActor)
{
    static auto InstigatingActorProperty = StaticStruct()->FindPropertyByName(FName(TEXT("InstigatingActor")));

    if (InstigatingActorProperty)
        *InstigatingActorProperty->ContainerPtrToValuePtr<AActor*>(this) = InstigatingActor;
}

UClass* UFortControllerEffect::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect"));

    return Class;
}

void UFortControllerEffect::ApplyEffect(AController* TargetController, const FEffectRequestContext& RequestContext) const
{
    static auto ApplyEffect = Anchor::Function<void (*)(const UFortControllerEffect*, AController*, const FEffectRequestContext&)>(TEXT("UFortControllerEffect::ApplyEffect"))
        .Native(TEXT("/Script/FortniteGame.FortControllerEffect:ApplyEffect"))
        .Get();

    if (auto ApplyGameplayEffectBase = Cast<UFortControllerEffect_ApplyGameplayEffectBase>(this))
        return ApplyGameplayEffectBase->ApplyGameplayEffect(TargetController);

    if (auto NPCGiveGift = Cast<UFortControllerEffect_NPCGiveGift>(this))
        return NPCGiveGift->GiveGift(TargetController, RequestContext);

    if (auto TriggerCustomFeedMessage = Cast<UFortControllerEffect_TriggerCustomFeedMessage>(this))
        return TriggerCustomFeedMessage->TriggerCustomFeedMessage(TargetController);

    ApplyEffect(this, TargetController, RequestContext);
}

UClass* UFortControllerEffect_ApplyGameplayEffectBase::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_ApplyGameplayEffectBase")) ? (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_ApplyGameplayEffectBase")) : UFortControllerEffect_ApplyGameplayEffect::StaticClass();

    return Class;
}

void UFortControllerEffect_ApplyGameplayEffectBase::ApplyGameplayEffect(AController* TargetController) const
{
    static auto GameplayEffect = StaticClass()->FindPropertyByName(FName(TEXT("GameplayEffect")));
    static auto EffectLevel = StaticClass()->FindPropertyByName(FName(TEXT("EffectLevel")));
    static auto ParameterName = StaticClass()->FindPropertyByName(FName(TEXT("ParameterName")));

    auto Pawn = TargetController ? Cast<AFortPawn>(TargetController->GetPawn()) : nullptr;
    auto AbilitySystemComponent = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
    auto GameplayEffectClass = *GameplayEffect->ContainerPtrToValuePtr<UClass*>(this);

    if (!AbilitySystemComponent || !GameplayEffectClass)
        return;

    auto SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, *EffectLevel->ContainerPtrToValuePtr<float>(this), AbilitySystemComponent->MakeEffectContext());

    if (!SpecHandle.IsValid())
        return;

    for (auto RestoreQuantityName : { TEXT("HealthRestoreQuantity"), TEXT("ShieldRestoreQuantity"), TEXT("ParameterQuantity") })
    {
        auto RestoreQuantity = GetClass()->FindPropertyByName(FName(RestoreQuantityName));

        if (RestoreQuantity)
            SpecHandle.Data->SetSetByCallerMagnitude(ParameterName->GetPropertyValue_InContainer<FGameplayTag>(this), RestoreQuantity->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));
    }

    AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

UClass* UFortControllerEffect_ApplyGameplayEffect::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_ApplyGameplayEffect"));

    return Class;
}

UClass* UFortControllerEffect_NPCGiveGift::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_NPCGiveGift"));

    return Class;
}

void UFortControllerEffect_NPCGiveGift::GiveGift(AController* TargetController, const FEffectRequestContext& RequestContext) const
{
    auto InstigatingController = const_cast<FEffectRequestContext&>(RequestContext).GetInstigatingController();
    auto NPCPawn = InstigatingController ? InstigatingController->GetPawn() : nullptr;
    auto NPCComponent = NPCPawn ? NPCPawn->FindComponentByClass<UFortNPCConversationParticipantComponent>() : nullptr;
    auto PlayerController = Cast<AFortPlayerController>(TargetController);

    if (NPCComponent && PlayerController)
        UFortConversationTaskNode_GiveGift::GiveGift(NPCComponent, PlayerController);
}

UClass* UFortControllerEffect_TriggerCustomFeedMessage::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_TriggerCustomFeedMessage"));

    return Class;
}

void UFortControllerEffect_TriggerCustomFeedMessage::TriggerCustomFeedMessage(AController* TargetController) const
{
    static auto CustomFeedMessageTag = StaticClass()->FindPropertyByName(FName(TEXT("CustomFeedMessageTag")));
    static auto AdditionalLocalSpaceContextOriginOffset = StaticClass()->FindPropertyByName(FName(TEXT("AdditionalLocalSpaceContextOriginOffset")));

    TriggerCustomFeedMessage(TargetController, CustomFeedMessageTag->GetPropertyValue_InContainer<FGameplayTag>(this), *AdditionalLocalSpaceContextOriginOffset->ContainerPtrToValuePtr<FVector>(this));
}

void UFortControllerEffect_TriggerCustomFeedMessage::TriggerCustomFeedMessage(AController* TargetController, const FGameplayTag& CustomFeedMessageTag, const FVector& LocalSpaceContextOriginOffset)
{
    static auto CustomFeedMessageComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateComponent_CustomFeedMessage"));
    static auto NetMulticastTriggerCustomFeedMessageWithContext = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateComponent_CustomFeedMessage:NetMulticastTriggerCustomFeedMessageWithContext"));

    struct FortPlayerStateComponent_CustomFeedMessage_eventNetMulticastTriggerCustomFeedMessageWithContext_Parms
    {
        FGameplayTag CustomFeedMessageGeneratorID;
        TWeakObjectPtr<APlayerState> OwningPlayerState;
        FVector OriginLocation;
    };

    auto PlayerState = TargetController ? TargetController->GetPlayerState() : nullptr;
    auto Pawn = TargetController ? TargetController->GetPawn() : nullptr;
    auto CustomFeedMessageComponent = PlayerState && CustomFeedMessageComponentClass ? PlayerState->FindComponentByClass(CustomFeedMessageComponentClass) : nullptr;

    if (!CustomFeedMessageComponent || !NetMulticastTriggerCustomFeedMessageWithContext)
        return;

    FortPlayerStateComponent_CustomFeedMessage_eventNetMulticastTriggerCustomFeedMessageWithContext_Parms Parms;
    Parms.CustomFeedMessageGeneratorID = CustomFeedMessageTag;
    Parms.OwningPlayerState = PlayerState;
    Parms.OriginLocation = Pawn ? Pawn->GetTransform().TransformPosition(LocalSpaceContextOriginOffset) : FVector(ForceInit);

    CustomFeedMessageComponent->CallRemoteFunction(NetMulticastTriggerCustomFeedMessageWithContext, &Parms, nullptr, nullptr);
}
