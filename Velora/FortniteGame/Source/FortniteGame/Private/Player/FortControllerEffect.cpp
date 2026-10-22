#include "pch.h"
#include "Player/FortControllerEffect.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "GameplayEffect.h"
#include "Pawns/FortPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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

    ApplyEffect(this, TargetController, RequestContext);
}

UClass* UFortControllerEffect_ApplyGameplayEffect::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerEffect_ApplyGameplayEffect"));

    return Class;
}

void UFortControllerEffect_ApplyGameplayEffect::ApplyGameplayEffect(AController* TargetController) const
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

    for (auto RestoreQuantityName : { TEXT("HealthRestoreQuantity"), TEXT("ShieldRestoreQuantity") })
    {
        auto RestoreQuantity = GetClass()->FindPropertyByName(FName(RestoreQuantityName));

        if (RestoreQuantity)
            SpecHandle.Data->SetSetByCallerMagnitude(*ParameterName->ContainerPtrToValuePtr<FGameplayTag>(this), RestoreQuantity->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f));
    }

    AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}
