#include "pch.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UAbilitySystemComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.AbilitySystemComponent"));

    return Class;
}

bool UAbilitySystemComponent::HasAttributeSetForAttribute(FGameplayAttribute Attribute) const
{
    static auto HasAttributeSetForAttribute = Anchor::Function<bool (*)(const UAbilitySystemComponent*, FGameplayAttribute)>(TEXT("UAbilitySystemComponent::HasAttributeSetForAttribute"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
        .Find("call 0x*; test al, al; jz 0x*")
        .Call()
        .Get();

    return HasAttributeSetForAttribute(this, Attribute);
}

void UAbilitySystemComponent::CaptureAttributeForGameplayEffect(FGameplayEffectAttributeCaptureSpec& OutCaptureSpec)
{
    static auto CaptureAttributeForGameplayEffect = Anchor::Function<void (*)(UAbilitySystemComponent*, FGameplayEffectAttributeCaptureSpec&)>(TEXT("UAbilitySystemComponent::CaptureAttributeForGameplayEffect"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
        .Find("movdqa [*], xmm0; call 0x*")
        .Call()
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
        .Find("call 0x*; test al, al; jz 0x*")
        .Find("call 0x*; lea rdx, [*]; mov rcx, *; call 0x*")
        .Call(2)
        .Get();

    CaptureAttributeForGameplayEffect(this, OutCaptureSpec);
}

FGameplayEffectContextHandle UAbilitySystemComponent::MakeEffectContext() const
{
    static auto VirtualMakeEffectContext = Anchor::VirtualFunction<FGameplayEffectContextHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectContextHandle*)>(TEXT("UAbilitySystemComponent::MakeEffectContext"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeEffectContext"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeEffectContext"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    FGameplayEffectContextHandle Context;

    if (VirtualMakeEffectContext)
    {
        VirtualMakeEffectContext(this, &Context);

        return Context;
    }

    static auto MakeEffectContext = Anchor::Function<FGameplayEffectContextHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectContextHandle*)>(TEXT("UAbilitySystemComponent::MakeEffectContext"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeEffectContext"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    MakeEffectContext(this, &Context);

    return Context;
}

FGameplayEffectSpecHandle UAbilitySystemComponent::MakeOutgoingSpec(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle Context) const
{
    static auto VirtualMakeOutgoingSpec = Anchor::VirtualFunction<FGameplayEffectSpecHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectSpecHandle*, UClass*, float, FGameplayEffectContextHandle)>(TEXT("UAbilitySystemComponent::MakeOutgoingSpec"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call [*]")
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov r9, [rax+0x*]")
        .Optional()
        .Get();

    FGameplayEffectSpecHandle SpecHandle;

    if (VirtualMakeOutgoingSpec)
    {
        VirtualMakeOutgoingSpec(this, &SpecHandle, GameplayEffectClass, Level, MoveTemp(Context));

        return SpecHandle;
    }

    static auto MakeOutgoingSpec = Anchor::Function<FGameplayEffectSpecHandle* (*)(const UAbilitySystemComponent*, FGameplayEffectSpecHandle*, UClass*, float, FGameplayEffectContextHandle)>(TEXT("UAbilitySystemComponent::MakeOutgoingSpec"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:MakeOutgoingSpec"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    MakeOutgoingSpec(this, &SpecHandle, GameplayEffectClass, Level, MoveTemp(Context));

    return SpecHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& Spec, FPredictionKey PredictionKey)
{
    static auto VirtualApplyGameplayEffectSpecToSelf = Anchor::VirtualFunction<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, const FGameplayEffectSpec&, FPredictionKey)>(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectSpecToSelf"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectSpecToSelf"))
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;

    if (VirtualApplyGameplayEffectSpecToSelf)
    {
        VirtualApplyGameplayEffectSpecToSelf(this, &ActiveGameplayEffectHandle, Spec, PredictionKey);

        return ActiveGameplayEffectHandle;
    }

    static auto ApplyGameplayEffectSpecToSelf = Anchor::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, const FGameplayEffectSpec&, FPredictionKey)>(TEXT("UAbilitySystemComponent::ApplyGameplayEffectSpecToSelf"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectSpecToSelf"))
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    ApplyGameplayEffectSpecToSelf(this, &ActiveGameplayEffectHandle, Spec, PredictionKey);

    return ActiveGameplayEffectHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::BP_ApplyGameplayEffectToTarget(TSubclassOf<UGameplayEffect> GameplayEffectClass, UAbilitySystemComponent* Target, float Level, FGameplayEffectContextHandle Context)
{
    static auto ApplyGameplayEffectToTarget = Anchor::Optional<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, UGameplayEffect*, UAbilitySystemComponent*, float, FGameplayEffectContextHandle, FPredictionKey)>(TEXT("UAbilitySystemComponent::ApplyGameplayEffectToTarget"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToTarget"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("movss [rsp+0x20], xmm*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (ApplyGameplayEffectToTarget)
    {
        FActiveGameplayEffectHandle ActiveGameplayEffectHandle;

        if (!Target || !GameplayEffectClass)
            return ActiveGameplayEffectHandle;

        if (!Context.IsValid())
            Context = MakeEffectContext();

        ApplyGameplayEffectToTarget(this, &ActiveGameplayEffectHandle, GameplayEffectClass.GetDefaultObject(), Target, Level, MoveTemp(Context), FPredictionKey());

        return ActiveGameplayEffectHandle;
    }

    static auto BP_ApplyGameplayEffectToTarget = Anchor::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, UClass*, UAbilitySystemComponent*, float, FGameplayEffectContextHandle)>(TEXT("UAbilitySystemComponent::BP_ApplyGameplayEffectToTarget"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToTarget"))
        .Get();

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    BP_ApplyGameplayEffectToTarget(this, &ActiveGameplayEffectHandle, GameplayEffectClass, Target, Level, MoveTemp(Context));

    return ActiveGameplayEffectHandle;
}

FActiveGameplayEffectHandle UAbilitySystemComponent::BP_ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level, FGameplayEffectContextHandle EffectContext)
{
    static auto BP_ApplyGameplayEffectToSelf = Anchor::Function<FActiveGameplayEffectHandle* (*)(UAbilitySystemComponent*, FActiveGameplayEffectHandle*, UClass*, float, FGameplayEffectContextHandle)>(TEXT("UAbilitySystemComponent::BP_ApplyGameplayEffectToSelf"))
        .Native(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:BP_ApplyGameplayEffectToSelf"))
        .Get();

    FActiveGameplayEffectHandle ActiveGameplayEffectHandle;
    BP_ApplyGameplayEffectToSelf(this, &ActiveGameplayEffectHandle, GameplayEffectClass, Level, MoveTemp(EffectContext));

    return ActiveGameplayEffectHandle;
}

void UAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
    static auto OnTagUpdated = Anchor::VirtualFunction<void (*)(UAbilitySystemComponent*, const FGameplayTag&, bool)>(TEXT("UAbilitySystemComponent::OnTagUpdated"))
        .String(TEXT("UFortAbilitySystemComponent::OnTagUpdated is updating the HealthRegenDelay tag. bTagExists: %s"))
        .Start()
        .Log(TEXT("UFortAbilitySystemComponent::OnTagUpdated"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAbilitySystemComponent"))
        .Get();

    OnTagUpdated(this, Tag, TagExists);
}

void UAbilitySystemComponent::RemoveGameplayCue(const FGameplayTag GameplayCueTag)
{
    RemoveGameplayCue_Internal(GameplayCueTag, GetActiveGameplayCues());
}

void UAbilitySystemComponent::RemoveGameplayCue_Internal(const FGameplayTag GameplayCueTag, FActiveGameplayCueContainer& GameplayCueContainer)
{
    static auto VirtualRemoveGameplayCue_Internal = Anchor::VirtualFunction<void (*)(UAbilitySystemComponent*, FGameplayTag, FActiveGameplayCueContainer&)>(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal"))
        .Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:K2_RemoveGameplayCue"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("UGameplayAbility::K2_RemoveGameplayCue"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__GameplayAbility"))
        .FindEither("lea r8, [*+0x*]; call *", "lea r8, [*+0x*]; mov rdx, *; call *")
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    if (VirtualRemoveGameplayCue_Internal)
        return VirtualRemoveGameplayCue_Internal(this, GameplayCueTag, GameplayCueContainer);

    static auto RemoveGameplayCue_Internal = Anchor::Function<void (*)(UAbilitySystemComponent*, FGameplayTag, FActiveGameplayCueContainer&)>(TEXT("UAbilitySystemComponent::RemoveGameplayCue_Internal"))
        .Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:K2_RemoveGameplayCue"))
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/GameplayAbilities.Default__GameplayAbility"))
        .FindEither("lea r8, [*+0x*]; call *", "lea r8, [*+0x*]; mov rdx, *; call *")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    RemoveGameplayCue_Internal(this, GameplayCueTag, GameplayCueContainer);
}

FActiveGameplayCueContainer& UAbilitySystemComponent::GetActiveGameplayCues()
{
    static auto ActiveGameplayCues = GetClass()->FindPropertyByName(FName(TEXT("ActiveGameplayCues")));

    return *ActiveGameplayCues->ContainerPtrToValuePtr<FActiveGameplayCueContainer>(this);
}

FGameplayTagCountContainer& UAbilitySystemComponent::GetGameplayTagCountContainer()
{
    static auto GameplayTagCountContainer = Anchor::Offset(TEXT("UAbilitySystemComponent::GameplayTagCountContainer"))
        .String(TEXT("Unexpected Gameplay.Action.Player.Sprint tag, attempting to recover (FORT-34412)"))
        .Start()
        .FindEither("lea rcx, [*+0x*]; * r8d, 0xffffffff", "lea rcx, [*+0x*]; mov *, *; lea rdx, [*+0x*]; lea r8d, [r9-0x1]")
        .Or()
        .String(TEXT("Unexpected Gameplay.Action.Player.Sprint tag, attempting to recover (FORT-34412)"))
        .Find("lea rcx, [*+0x*]; or r8d, 0xffffffff")
        .Disp();

    return *(FGameplayTagCountContainer*)((uint8_t*)this + GameplayTagCountContainer);
}
