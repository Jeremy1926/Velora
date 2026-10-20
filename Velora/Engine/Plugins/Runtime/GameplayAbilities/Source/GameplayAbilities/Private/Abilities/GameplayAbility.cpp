#include "pch.h"
#include "Abilities/GameplayAbility.h"
#include "Scanner.h"

AActor* UGameplayAbility::GetAvatarActorFromActorInfo() const
{
    static auto GetAvatarActorFromActorInfo = Anchor::Function<AActor* (*)(const UGameplayAbility*)>(TEXT("UGameplayAbility::GetAvatarActorFromActorInfo"))
        .Native(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo"))
        .Get();

    return GetAvatarActorFromActorInfo(this);
}
