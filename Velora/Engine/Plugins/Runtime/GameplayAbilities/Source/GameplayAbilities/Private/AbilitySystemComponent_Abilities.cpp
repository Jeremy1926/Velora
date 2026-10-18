#include "pch.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

struct AbilitySystemComponent_eventClientActivateAbilityFailed_Parms
{
    FGameplayAbilitySpecHandle AbilityToActivate;
    int16 PredictionKey;
};

FGameplayTagContainer& UAbilitySystemComponent::InternalTryActivateAbilityFailureTags = *(FGameplayTagContainer*)Scanner::String(TEXT("InternalTryActivateAbility called with invalid Ability")).Start().Find("lea rcx, [0x*]").Target(TEXT("UAbilitySystemComponent::InternalTryActivateAbilityFailureTags"));

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbility(const FGameplayAbilitySpec& Spec)
{
    static auto GiveAbility = (FGameplayAbilitySpecHandle* (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, const FGameplayAbilitySpec&))Scanner::String(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!")).Call(3).Get(TEXT("UAbilitySystemComponent::GiveAbility"));

    static auto InternalServerTryActiveAbility = [this]
    {
        auto VTable = *(uint8_t**)this;
        auto ServerTryActivateAbility_Implementation = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerTryActivateAbility")).Find("call [*]", 2).Slot(TEXT("UAbilitySystemComponent::ServerTryActivateAbility_Implementation"));
        auto InternalServerTryActiveAbility = Scanner::At(*(const uint8_t**)(VTable + ServerTryActivateAbility_Implementation)).Find("call [*]").Slot(TEXT("UAbilitySystemComponent::InternalServerTryActiveAbility"));

        return DetourVTable(TEXT("UAbilitySystemComponent::InternalServerTryActiveAbility"), (void**)(VTable + InternalServerTryActiveAbility), &UAbilitySystemComponent::InternalServerTryActiveAbility);
    }();

    FGameplayAbilitySpecHandle Handle;

    GiveAbility(this, &Handle, Spec);

    return Handle;
}

FGameplayAbilitySpec* UAbilitySystemComponent::FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().Items)
    {
        if (Spec.Handle == Handle)
        {
            return &Spec;
        }
    }

    return nullptr;
}

FGameplayAbilitySpec* UAbilitySystemComponent::FindAbilitySpecFromClass(TSubclassOf<UGameplayAbility> InAbilityClass)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().Items)
    {
        if (Spec.Ability->GetClass() == InAbilityClass)
        {
            return &Spec;
        }
    }

    return nullptr;
}

void UAbilitySystemComponent::MarkAbilitySpecDirty(FGameplayAbilitySpec& Spec)
{
    static auto MarkAbilitySpecDirty = (void (*)(UAbilitySystemComponent*, FGameplayAbilitySpec&))Scanner::String(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!")).Call(3).Find("ret").Last("call 0x*").Call().Get(TEXT("UAbilitySystemComponent::MarkAbilitySpecDirty"));

    MarkAbilitySpecDirty(this, Spec);
}

bool UAbilitySystemComponent::InternalTryActivateAbility(FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
    static auto InternalTryActivateAbility = (bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey*, UGameplayAbility**, FOnGameplayAbilityEnded::FDelegate*, const FGameplayEventData*))Scanner::String(TEXT("InternalTryActivateAbility called with invalid Ability")).Start().Get(TEXT("UAbilitySystemComponent::InternalTryActivateAbility"));

    return InternalTryActivateAbility(this, Handle, &InPredictionKey, OutInstancedAbility, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UAbilitySystemComponent::ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16 PredictionKey)
{
    static auto ClientActivateAbilityFailed = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ClientActivateAbilityFailed"));

    AbilitySystemComponent_eventClientActivateAbilityFailed_Parms Parms;
    Parms.AbilityToActivate = AbilityToActivate;
    Parms.PredictionKey = PredictionKey;

    CallRemoteFunction(ClientActivateAbilityFailed, &Parms, nullptr, nullptr);
}

void UAbilitySystemComponent::InternalServerTryActiveAbility(FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
{
    FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
    if (!Spec)
    {
        ABILITY_LOG(Display, TEXT("InternalServerTryActiveAbility. Rejecting ClientActivation of ability with invalid SpecHandle!"));
        ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        return;
    }

    ConsumeAllReplicatedData(Handle, PredictionKey);

    FScopedPredictionWindow ScopedPredictionWindow(this, PredictionKey);

    UGameplayAbility* InstancedAbility = nullptr;
    Spec->InputPressed = true;

    if (!InternalTryActivateAbility(Handle, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
    {
        ABILITY_LOG(Display, TEXT("InternalServerTryActiveAbility. Rejecting ClientActivation of %s. InternalTryActivateAbility failed: %s"), *GetNameSafe(Spec->Ability), *InternalTryActivateAbilityFailureTags.ToStringSimple());
        ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
    }
    MarkAbilitySpecDirty(*Spec);
}

void UAbilitySystemComponent::ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey)
{
    FAbilityReplicatedDataCache* CachedData = GetAbilityTargetDataMap().Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, AbilityOriginalPredictionKey));
    if (CachedData)
    {
        CachedData->Reset();
    }
}

FGameplayAbilitySpecContainer& UAbilitySystemComponent::GetActivatableAbilities()
{
    static auto ActivatableAbilities = GetClass()->FindPropertyByName(FName(TEXT("ActivatableAbilities")));

    return *ActivatableAbilities->ContainerPtrToValuePtr<FGameplayAbilitySpecContainer>(this);
}

TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>& UAbilitySystemComponent::GetAbilityTargetDataMap()
{
    static auto AbilityTargetDataMap = Scanner::String(TEXT("Ability %s is overriding pending replicated target data.")).Start().Find("lea *, [*+0x???]").Disp(TEXT("UAbilitySystemComponent::AbilityTargetDataMap"));

    return *(TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>*)((uint8_t*)this + AbilityTargetDataMap);
}
