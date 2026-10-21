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

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbility(const FGameplayAbilitySpec& Spec)
{
    static auto GiveAbility = Anchor::Function<FGameplayAbilitySpecHandle* (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, const FGameplayAbilitySpec&)>(TEXT("UAbilitySystemComponent::GiveAbility"))
        .Smallest(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"))
        .Find("lea rdx, [rbp+0x*]")
        .Call()
        .Or()
        .Smallest(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"))
        .Call()
        .Get();

    static auto InternalServerTryActiveAbility = Anchor::Hook(TEXT("UAbilitySystemComponent::InternalServerTryActiveAbility"), &UAbilitySystemComponent::InternalServerTryActiveAbility)
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerTryActivateAbility"))
        .Find("call [*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerTryActivateAbility_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Find("call [*]")
        .LogSlot(TEXT("UAbilitySystemComponent::InternalServerTryActiveAbility"))
        .InVTable(*(void***)this);

    FGameplayAbilitySpecHandle Handle;

    GiveAbility(this, &Handle, Spec);

    return Handle;
}

FGameplayAbilitySpecHandle UAbilitySystemComponent::GiveAbilityAndActivateOnce(const FGameplayAbilitySpec& Spec)
{
    static auto GiveAbilityAndActivateOnce = Anchor::Function<FGameplayAbilitySpecHandle* (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, const FGameplayAbilitySpec&)>(TEXT("UAbilitySystemComponent::GiveAbilityAndActivateOnce"))
        .Smallest(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"))
        .Start()
        .Get();

    FGameplayAbilitySpecHandle Handle;

    GiveAbilityAndActivateOnce(this, &Handle, Spec);

    return Handle;
}

FGameplayAbilitySpec* UAbilitySystemComponent::FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle)
{
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().GetItems())
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
    for (FGameplayAbilitySpec& Spec : GetActivatableAbilities().GetItems())
    {
        if (Spec.Ability->GetClass() == InAbilityClass)
        {
            return &Spec;
        }
    }

    return nullptr;
}

void UAbilitySystemComponent::MarkAbilitySpecDirty(FGameplayAbilitySpec& Spec, bool WasAddOrRemove)
{
    static auto MarkAbilitySpecDirty = Anchor::Function<void (*)(UAbilitySystemComponent*, FGameplayAbilitySpec&, bool)>(TEXT("UAbilitySystemComponent::MarkAbilitySpecDirty"))
        .Smallest(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"))
        .Find("lea rdx, [rbp+0x*]")
        .Call()
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Or()
        .Smallest(TEXT("GiveAbilityAndActivateOnce called on ability %s on the client, not allowed!"))
        .Call()
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Get();

    MarkAbilitySpecDirty(this, Spec, WasAddOrRemove);
}

bool UAbilitySystemComponent::InternalTryActivateAbility(FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData)
{
    static auto InternalTryActivateAbility = Anchor::Function<bool (*)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey*, UGameplayAbility**, FOnGameplayAbilityEnded::FDelegate*, const FGameplayEventData*)>(TEXT("UAbilitySystemComponent::InternalTryActivateAbility"))
        .String(TEXT("InternalTryActivateAbility called with invalid Ability"))
        .Start()
        .Get();

    return InternalTryActivateAbility(this, Handle, &InPredictionKey, OutInstancedAbility, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

FGameplayTagContainer& UAbilitySystemComponent::GetInternalTryActivateAbilityFailureTags()
{
    static auto InternalTryActivateAbilityFailureTags = Anchor::Optional<FGameplayTagContainer*>(TEXT("UAbilitySystemComponent::InternalTryActivateAbilityFailureTags"))
        .String(TEXT("InternalTryActivateAbility called with invalid Ability"))
        .Start()
        .Find("xor edx, edx")
        .AdjacentOrLast("lea rcx, [*]", "lea *, [*]")
        .Is("lea *, [0x*]")
        .Target()
        .Get();

    if (InternalTryActivateAbilityFailureTags)
        return *InternalTryActivateAbilityFailureTags;

    static auto Offset = Anchor::Offset(TEXT("UAbilitySystemComponent::InternalTryActivateAbilityFailureTags"))
        .String(TEXT("InternalTryActivateAbility called with invalid Ability"))
        .Start()
        .Find("xor edx, edx")
        .AdjacentOrLast("lea rcx, [*]", "lea *, [*]")
        .Disp();

    return *(FGameplayTagContainer*)((uint8_t*)this + Offset);
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
        ABILITY_LOG(Display, TEXT("InternalServerTryActiveAbility. Rejecting ClientActivation of %s. InternalTryActivateAbility failed: %s"), *GetNameSafe(Spec->Ability), *GetInternalTryActivateAbilityFailureTags().ToStringSimple());
        ClientActivateAbilityFailed(Handle, PredictionKey.Current);
        Spec->InputPressed = false;
    }
    MarkAbilitySpecDirty(*Spec);
}

void UAbilitySystemComponent::ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey)
{
    static auto bReplicatedDataContainer = !Anchor::Exists()
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("imul *, *, 0x*")
        .Get();

    auto& AbilityTargetDataMap = GetAbilityTargetDataMap();
    FAbilityReplicatedDataCache* CachedData = bReplicatedDataContainer ? ((FGameplayAbilityReplicatedDataContainer&)AbilityTargetDataMap).Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, AbilityOriginalPredictionKey)) : AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, AbilityOriginalPredictionKey));
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

void UAbilitySystemComponent::CancelAbilities(const FGameplayTagContainer* WithTags, const FGameplayTagContainer* WithoutTags, UGameplayAbility* Ignore)
{
    static auto CancelAbilities = Anchor::Function<void (*)(UAbilitySystemComponent*, const FGameplayTagContainer*, const FGameplayTagContainer*, UGameplayAbility*)>(TEXT("UAbilitySystemComponent::CancelAbilities"))
        .String(TEXT("Unexpected Gameplay.Action.Player.Sprint tag, attempting to recover (FORT-34412)"))
        .Start()
        .Find("xor r9d, r9d; lea rdx, *; xor r8d, r8d; mov rcx, *; call 0x*")
        .Call()
        .Get();

    CancelAbilities(this, WithTags, WithoutTags, Ignore);
}

int32 UAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
    static auto HandleGameplayEvent = Anchor::VirtualFunction<int32 (*)(UAbilitySystemComponent*, FGameplayTag, const FGameplayEventData*)>(TEXT("UAbilitySystemComponent::HandleGameplayEvent"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:SendGameplayEventToActor"))
        .Find("mov rcx, [rsp+0x*]; call 0x*")
        .Call()
        .Find("call [*+0x*]", 2)
        .Get();

    return HandleGameplayEvent(this, EventTag, Payload);
}

TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>& UAbilitySystemComponent::GetAbilityTargetDataMap()
{
    static auto AbilityTargetDataMap = Anchor::Offset(TEXT("UAbilitySystemComponent::AbilityTargetDataMap"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("lea *, [*+0x???]")
        .Disp();

    return *(TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>*)((uint8_t*)this + AbilityTargetDataMap);
}

FAbilityReplicatedDataCache* TMap<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>::Find(const FGameplayAbilitySpecHandleAndPredictionKey& Key)
{
    static auto ElementSize = (int32)Anchor::Offset(TEXT("TSetElement<TPair<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>>"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("imul *, *, 0x*")
        .Imm();

    static auto HashNextId = Anchor::Offset(TEXT("TSetElement<TPair<FGameplayAbilitySpecHandleAndPredictionKey, FAbilityReplicatedDataCache>>::HashNextId"))
        .String(TEXT("Ability %s is overriding pending replicated target data."))
        .Start()
        .Find("imul *, *, 0x*")
        .Find("mov *, [*+0x*]; cmp *, 0xffffffff")
        .Disp();

    if (Elements.Num() == NumFreeIndices)
        return nullptr;

    for (FSetElementId ElementId = Hash.GetAllocation()[GetTypeHash(Key) & (HashSize - 1)]; ElementId.IsValidId(); ElementId = *(FSetElementId*)(Elements.GetData() + ElementId * ElementSize + HashNextId))
    {
        if (*(FGameplayAbilitySpecHandleAndPredictionKey*)(Elements.GetData() + ElementId * ElementSize) == Key)
            return (FAbilityReplicatedDataCache*)(Elements.GetData() + ElementId * ElementSize + ElementSize - HashNextId);
    }

    return nullptr;
}

FAbilityReplicatedDataCache* FGameplayAbilityReplicatedDataContainer::Find(const FGameplayAbilitySpecHandleAndPredictionKey& Key)
{
    for (const FKeyDataPair& Pair : InUseData)
    {
        if (Pair.Key == Key)
        {
            return &Pair.Value.Get();
        }
    }

    return nullptr;
}
