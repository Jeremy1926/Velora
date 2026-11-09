#include "pch.h"
#include "Weapons/FortWeaponRanged.h"
#include "Abilities/FortGameplayAbility_RangedWeapon.h"
#include "Pawns/FortPawn.h"
#include "AbilitySystemComponent.h"
#include "Engine/EngineTypes.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortWeaponRanged::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponRanged"));

    return Class;
}

void AFortWeaponRanged::ServerNotifyPawnHit_Implementation(const FHitResult& Hit, const TGameValue<FVector>& ProjectileOriginPosition, float ProjectileStartTimestamp, const TArray<uint8>& Context)
{
    DETOUR(AFortWeaponRanged, ServerNotifyPawnHit_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponRanged:ServerNotifyPawnHit"))
        .Optional()
        .Find("add [*+0x20], *")
        .Find("mov r9, [rax+0x*]")
        .LogSlot(TEXT("AFortWeaponRanged::ServerNotifyPawnHit_Implementation"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortWeaponRanged"), "ret*")
        .Optional()
        .InChildVTables(StaticClass()));

    static auto TargetDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayAbilities.GameplayAbilityTargetDataHandle"));

    auto Pawn = Cast<AFortPawn>(GetInstigator());
    auto AbilitySystemComponent = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
    auto Ability = Cast<UFortGameplayAbility_RangedWeapon>(GetActiveAbility());

    if (!Ability && AbilitySystemComponent)
    {
        auto Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(GetPrimaryAbilitySpecHandle());

        Ability = Spec ? Cast<UFortGameplayAbility_RangedWeapon>(Spec->GetPrimaryInstance()) : nullptr;
    }

    if (!Ability || !HasAuthority())
        return;

    TArray<uint8> TargetData;
    TargetData.AddUninitialized(TargetDataStruct->GetPropertiesSize());

    FMemory::Memzero(TargetData.GetData(), TargetData.Num());

    Ability->MakeTargetDataHandleFromHitResults(*(FGameplayAbilityTargetDataHandle*)TargetData.GetData(), Hit);
    Ability->ApplyGameplayEffectContainer(*(FGameplayAbilityTargetDataHandle*)TargetData.GetData(), FGameplayTag(), (int32)Ability->GetCurrentAbilitySourceLevel());
}
