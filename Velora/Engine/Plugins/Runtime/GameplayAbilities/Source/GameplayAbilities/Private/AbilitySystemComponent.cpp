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

    FGameplayEffectContextHandle Context;
    (Slot ? Scanner::Virtual<decltype(MakeEffectContext)>(this, Slot) : MakeEffectContext)(this, &Context);

    return Context;
}

FGameplayEffectSpecHandle UAbilitySystemComponent::MakeOutgoingSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle Context) const
{
    static auto Call = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec")).Find("setnz *").Find("mov [*+0x20], *").Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::MakeOutgoingSpec")) : 0;
    static auto MakeOutgoingSpec = Scanner::Function<FGameplayEffectSpecHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectSpecHandle*, UClass*, float, FGameplayEffectContextHandle)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::MakeOutgoingSpec")));

    FGameplayEffectSpecHandle SpecHandle;
    (Slot ? Scanner::Virtual<decltype(MakeOutgoingSpec)>(this, Slot) : MakeOutgoingSpec)(this, &SpecHandle, GameplayEffectClass, Level, MoveTemp(Context));

    return SpecHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& Spec, FPredictionKey PredictionKey)
{
    static auto Call = Scanner::Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectSpecToSelf")).Find("call *");
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf")) : 0;
    static auto ApplyGameplayEffectSpecToSelf = Scanner::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, const FGameplayEffectSpec&, FPredictionKey)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf")));

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    (Slot ? Scanner::Virtual<decltype(ApplyGameplayEffectSpecToSelf)>(this, Slot) : ApplyGameplayEffectSpecToSelf)(this, &ActiveGameplayEffectHandle, Spec, PredictionKey);

    return ActiveGameplayEffectHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::BP_ApplyGameplayEffectToTarget(TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level, FGameplayEffectContextHandle Context)
{
    static auto BP_ApplyGameplayEffectToTarget = Anchor::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, UClass*, UAbilitySystemComponent*, float, FGameplayEffectContextHandle)>(TEXT("UAbilitySystemComponent::BP_ApplyGameplayEffectToTarget"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToTarget"))
        .Get();

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    BP_ApplyGameplayEffectToTarget(this, &ActiveGameplayEffectHandle, GameplayEffectClass, Target, Level, MoveTemp(Context));

    return ActiveGameplayEffectHandle;
}

void UAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
    static auto OnTagUpdated = Anchor::Slot(TEXT("UAbilitySystemComponent::OnTagUpdated"))
        .String(TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the HealthRegenDelay tag. bTagExists: %s"))
        .Start()
        .Log(TEXT("UFortAbilitySystemComponent::OnTagUpdated"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAbilitySystemComponent"))
        .Get();

    Scanner::Virtual<void (*)(UAbilitySystemComponent*, const FGameplayTag&, bool)>(this, OnTagUpdated)(this, Tag, TagExists);
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
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__GameplayAbility"));

        auto Body = Scanner::At(Scanner::Virtual<const void*>(DefaultObject, K2_RemoveGameplayCue));
        auto Call = Body.Find("lea r8, [*+0x*]; call *");

        return (Call.Get() ? Call : Body.Find("lea r8, [*+0x*]; mov rdx, *; call *")).Find("call *");
    }();
    static auto Slot = Call.Is("call [*]") ? Call.Slot(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal")) : 0;
    static auto RemoveGameplayCue_Internal = Scanner::Function<void (*)(UAbilitySystemComponent*, FGameplayTag, FActiveGameplayCueContainer&)>(Slot ? nullptr : Call.Target(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal")));

    (Slot ? Scanner::Virtual<decltype(RemoveGameplayCue_Internal)>(this, Slot) : RemoveGameplayCue_Internal)(this, GameplayCueTag, GameplayCueContainer);
}

FActiveGameplayCueContainer& UAbilitySystemComponent::GetActiveGameplayCues()
{
    static auto ActiveGameplayCues = GetClass()->FindPropertyByName(FName(TEXT("ActiveGameplayCues")));

    return *ActiveGameplayCues->ContainerPtrToValuePtr<FActiveGameplayCueContainer>(this);
}

FGameplayTagCountContainer& UAbilitySystemComponent::GetGameplayTagCountContainer()
{
    static auto GameplayTagCountContainer = []
    {
        auto RecoverSprint = Scanner::String(TEXT("Unexpected Gameplay.Action.Player.Sprint tag, attempting to recover (FORT-34412)")).Start();
        auto UpdateTagMap = RecoverSprint.Find("lea rcx, [*+0x*]; * r8d, 0xffffffff");

        return (UpdateTagMap.Get() ? UpdateTagMap : RecoverSprint.Find("lea rcx, [*+0x*]; mov *, *; lea rdx, [*+0x*]; lea r8d, [r9-0x1]")).Disp(TEXT("UAbilitySystemComponent::GameplayTagCountContainer"));
    }();

    return *(FGameplayTagCountContainer*)((uint8_t*)this + GameplayTagCountContainer);
}
