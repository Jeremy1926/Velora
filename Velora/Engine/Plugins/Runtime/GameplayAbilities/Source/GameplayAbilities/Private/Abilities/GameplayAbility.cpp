#include "pch.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

AActor* UGameplayAbility::GetAvatarActorFromActorInfo() const
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("mov rcx, [rcx+0x*]; test rcx, rcx; jz *; add rcx, 0x*; call 0x*")
        .Get();

    if (bInlined)
    {
        static auto CurrentActorInfo = Anchor::Offset(TEXT("UGameplayAbility::CurrentActorInfo"))
            .Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("mov rcx, [rcx+0x*]; test rcx, rcx; jz *; add rcx, 0x*; call 0x*")
            .Disp();

        static auto AvatarActor = Anchor::Offset(TEXT("FGameplayAbilityActorInfo::AvatarActor"))
            .Exec(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo"))
            .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
            .Find("mov rcx, [rcx+0x*]; test rcx, rcx; jz *; add rcx, 0x*; call 0x*")
            .Find("add rcx, 0x*")
            .Imm();

        if (!CurrentActorInfo || !AvatarActor)
            return nullptr;

        auto ActorInfo = *(const uint8**)((const uint8*)this + CurrentActorInfo);

        return ActorInfo ? (AActor*)((const FWeakObjectPtr*)(ActorInfo + AvatarActor))->Get() : nullptr;
    }

    static auto GetAvatarActorFromActorInfo = Anchor::Function<AActor* (*)(const UGameplayAbility*)>(TEXT("UGameplayAbility::GetAvatarActorFromActorInfo"))
        .Native(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo"))
        .Get();

    return GetAvatarActorFromActorInfo(this);
}
