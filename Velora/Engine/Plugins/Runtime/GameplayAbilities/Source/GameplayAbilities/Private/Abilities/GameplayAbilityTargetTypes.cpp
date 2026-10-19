#include "pch.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FAbilityReplicatedDataCache::Reset()
{
    static auto Reset = []
    {
        auto ServerSetReplicatedTargetDataCancelled_Implementation = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled")).Find("call [*]", 2).Slot(TEXT("UAbilitySystemComponent::ServerSetReplicatedTargetDataCancelled_Implementation"));
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"));

        return Scanner::Function<void (*)(FAbilityReplicatedDataCache*)>(Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ServerSetReplicatedTargetDataCancelled_Implementation)).Call(3).Get(TEXT("FAbilityReplicatedDataCache::Reset")));
    }();

    Reset(this);
}
