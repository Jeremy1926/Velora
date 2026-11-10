#include "pch.h"
#include "Abilities/FortGameplayAbility_RangedWeapon.h"
#include "Engine/EngineTypes.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortGameplayAbility_RangedWeapon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbility_RangedWeapon"));

    return Class;
}

void UFortGameplayAbility_RangedWeapon::MakeTargetDataHandleFromHitResults(FGameplayAbilityTargetDataHandle& OutTargetData, const FHitResult& HitResult)
{
    static auto MakeTargetDataHandleFromHitResults = Anchor::Function<void (*)(UFortGameplayAbility_RangedWeapon*, FGameplayAbilityTargetDataHandle*, const void*)>(TEXT("UFortGameplayAbility_RangedWeapon::MakeTargetDataHandleFromHitResults"))
        .String(TEXT("STAT_FortGA_Ranged_OnTargetDataReadyCB"))
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("STAT_FortGA_Ranged_OnTargetDataReadyCB"))
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("STAT_FortGA_Ranged_OnTargetDataReadyCB"))
        .Find("lea r8, [rsp+0x*]; lea rdx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    struct
    {
        const FHitResult* Data;
        int32 ArrayNum;
        int32 ArrayMax;
    } HitResults{ &HitResult, 1, 1 };

    MakeTargetDataHandleFromHitResults(this, &OutTargetData, &HitResults);
}
