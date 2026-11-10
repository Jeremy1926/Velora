#include "pch.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortGameplayAbility.h"
#include "GameFramework/Actor.h"
#include "GameplayAbilitySpec.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAbilitySystemComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAbilitySystemComponent"));

    return Class;
}

void UFortAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
    DETOUR(UFortAbilitySystemComponent, OnTagUpdated, Anchor::Hook()
        .String(TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the HealthRegenDelay tag. bTagExists: %s"))
        .Start()
        .Optional()
        .Inline());

    Original(Tag, TagExists);

    auto Owner = GetOwner();

    if (!Owner || !Owner->HasAuthority())
        return;

    const FGameplayTag UpdatedTag = TGameLayout<FGameplayTag>::Load(&Tag);
    TArray<UFortGameplayAbility*> ActiveAbilities;

    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().GetItems())
    {
        if (!Spec.ActiveCount)
            continue;

        for (auto Instance : Spec.GetAbilityInstances())
        {
            if (auto FortAbility = Cast<UFortGameplayAbility>(Instance))
                ActiveAbilities.Add(FortAbility);
        }
    }

    for (auto FortAbility : ActiveAbilities)
        FortAbility->TagChangedActionCallback(UpdatedTag, TagExists);
}
