#include "pch.h"
#include "Building/BuildingGameplayActor.h"
#include "GameplayEffectTypes.h"
#include "Scanner.h"

void ABuildingGameplayActor::K2_ExecuteGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
    static auto K2_ExecuteGameplayCue = Anchor::VirtualFunction<void (*)(ABuildingGameplayActor*, FGameplayTag, FGameplayEffectContextHandle)>(TEXT("ABuildingGameplayActor::K2_ExecuteGameplayCue"))
        .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActor:K2_ExecuteGameplayCue"))
        .Find("add [*+0x20], *")
        .Find("mov r9, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingGameplayActor:K2_ExecuteGameplayCue"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("mov r9, [rax+0x*]")
        .Get();

    if (K2_ExecuteGameplayCue)
        K2_ExecuteGameplayCue(this, GameplayCueTag, std::move(Context));
}
