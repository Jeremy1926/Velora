#include "pch.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FAbilityReplicatedDataCache::Reset()
{
    static auto Reset = []
    {
        auto ServerSetReplicatedTargetDataCancelled_Implementation = Scanner::Exec(TEXT("/Script/GameplayAbilities.AbilitySystemComponent:ServerSetReplicatedTargetDataCancelled")).Find("call [*]", 2).Disp();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.Default__AbilitySystemComponent"));

        return (void (*)(FAbilityReplicatedDataCache*))Scanner::At(VTable[ServerSetReplicatedTargetDataCancelled_Implementation / sizeof(void*)]).Call(3).Get();
    }();

    Reset(this);
}
