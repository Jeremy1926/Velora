#include "pch.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FAbilityReplicatedDataCache::Reset()
{
    static auto Reset = Anchor::Function<void (*)(FAbilityReplicatedDataCache*)>(TEXT("FAbilityReplicatedDataCache::Reset"))
        .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled"))
        .Find("call [rax+0x*]", 2)
        .LogSlot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"))
        .Entry(TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"))
        .Call(3)
        .Get();

    Reset(this);
}
