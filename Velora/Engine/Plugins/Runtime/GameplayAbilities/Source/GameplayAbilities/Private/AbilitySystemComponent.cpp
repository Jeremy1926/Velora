#include "pch.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FGameplayEffectContextHandle UAbilitySystemComponent::MakeEffectContext() const
{
    static auto Call = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeEffectContext")).Find("setnz *").Find("mov [*+0x20], *").Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::MakeEffectContext")) : 0;
    static auto MakeEffectContext = Scanner::Function<FGameplayEffectContextHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectContextHandle*)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::MakeEffectContext")));

    auto VTable = *(uint8_t**)this;

    FGameplayEffectContextHandle Context;
    (Slot ? *(decltype(MakeEffectContext)*)(VTable + Slot) : MakeEffectContext)(this, &Context);

    return Context;
}

FGameplayEffectSpecHandle UAbilitySystemComponent::MakeOutgoingSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle Context) const
{
    static auto Call = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec")).Find("setnz *").Find("mov [*+0x20], *").Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::MakeOutgoingSpec")) : 0;
    static auto MakeOutgoingSpec = Scanner::Function<FGameplayEffectSpecHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectSpecHandle*, UClass*, float, FGameplayEffectContextHandle)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::MakeOutgoingSpec")));

    auto VTable = *(uint8_t**)this;

    FGameplayEffectSpecHandle SpecHandle;
    (Slot ? *(decltype(MakeOutgoingSpec)*)(VTable + Slot) : MakeOutgoingSpec)(this, &SpecHandle, GameplayEffectClass, Level, MoveTemp(Context));

    return SpecHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& Spec, FPredictionKey PredictionKey)
{
    static auto Call = Scanner::Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectSpecToSelf")).Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf")) : 0;
    static auto ApplyGameplayEffectSpecToSelf = Scanner::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, const FGameplayEffectSpec&, FPredictionKey)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf")));

    auto VTable = *(uint8_t**)this;

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    (Slot ? *(decltype(ApplyGameplayEffectSpecToSelf)*)(VTable + Slot) : ApplyGameplayEffectSpecToSelf)(this, &ActiveGameplayEffectHandle, Spec, PredictionKey);

    return ActiveGameplayEffectHandle;
}

void UAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
    static auto OnTagUpdated = []
    {
        auto FortOnTagUpdated = Scanner::String(TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the HealthRegenDelay tag. bTagExists: %s")).Start().Get(TEXT("UFortAbilitySystemComponent::OnTagUpdated"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortAbilitySystemComponent"));
        auto Index = 0;

        while (VTable[Index] != FortOnTagUpdated)
            Index++;

        return Scanner::Slot(TEXT("UAbilitySystemComponent::OnTagUpdated"), Index * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(UAbilitySystemComponent*, const FGameplayTag&, bool))(VTable + OnTagUpdated))(this, Tag, TagExists);
}

void UAbilitySystemComponent::RemoveGameplayCue(const FGameplayTag GameplayCueTag)
{
    RemoveGameplayCue_Internal(GameplayCueTag, GetActiveGameplayCues());
}

void UAbilitySystemComponent::RemoveGameplayCue_Internal(const FGameplayTag GameplayCueTag, FActiveGameplayCueContainer& GameplayCueContainer)
{
    static auto Call = []
    {
        auto K2_RemoveGameplayCue = Scanner::Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:K2_RemoveGameplayCue")).Find("call [*+0x*]").Slot(TEXT("UGameplayAbility::K2_RemoveGameplayCue"));
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__GameplayAbility"));

        return Scanner::At(VTable[K2_RemoveGameplayCue / sizeof(void*)]).Find("lea r8, [*+0x*]; call *").Find("call *");
    }();
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal")) : 0;
    static auto RemoveGameplayCue_Internal = Scanner::Function<void (*)(UAbilitySystemComponent*, FGameplayTag, FActiveGameplayCueContainer&)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal")));

    auto VTable = *(uint8_t**)this;

    (Slot ? *(decltype(RemoveGameplayCue_Internal)*)(VTable + Slot) : RemoveGameplayCue_Internal)(this, GameplayCueTag, GameplayCueContainer);
}

FActiveGameplayCueContainer& UAbilitySystemComponent::GetActiveGameplayCues()
{
    static auto ActiveGameplayCues = GetClass()->FindPropertyByName(FName(TEXT("ActiveGameplayCues")));

    return *ActiveGameplayCues->ContainerPtrToValuePtr<FActiveGameplayCueContainer>(this);
}

FGameplayTagCountContainer& UAbilitySystemComponent::GetGameplayTagCountContainer()
{
    static auto GameplayTagCountContainer = Scanner::String(TEXT("Unexpected Gameplay.Action.Player.Sprint tag, attempting to recover (FORT-34412)")).Start().Find("lea rcx, [*+0x*]; * r8d, 0xffffffff").Disp(TEXT("UAbilitySystemComponent::GameplayTagCountContainer"));

    return *(FGameplayTagCountContainer*)((uint8_t*)this + GameplayTagCountContainer);
}
