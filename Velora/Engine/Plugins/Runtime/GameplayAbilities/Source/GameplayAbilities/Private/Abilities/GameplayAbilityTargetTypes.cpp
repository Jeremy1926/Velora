#include "pch.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FAbilityReplicatedDataCache::Reset()
{
    static auto Reset = Anchor::Function<void (*)(FAbilityReplicatedDataCache*)>(TEXT("FAbilityReplicatedDataCache::Reset"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(3)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(3)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Find("call [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(3)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Find("add [*+0x20], *")
        .Find("mov rdi, [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(4)
        .Or()
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Has("bt rax, 0x8")
        .Find("setnz *; add r*, r*")
        .Find("call [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(3)
        .Get();

    Reset(this);
}
