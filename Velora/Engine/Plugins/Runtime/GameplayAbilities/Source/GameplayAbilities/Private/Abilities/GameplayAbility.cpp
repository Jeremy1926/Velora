#include "pch.h"
#include "Abilities/GameplayAbility.h"
#include "Scanner.h"

AActor* UGameplayAbility::GetAvatarActorFromActorInfo() const
{
    static auto GetAvatarActorFromActorInfo = Scanner::Function<AActor* (*)(const UGameplayAbility*)>(Scanner::Native(TEXT("/Script/GameplayAbilities.GameplayAbility:GetAvatarActorFromActorInfo")).Get(TEXT("UGameplayAbility::GetAvatarActorFromActorInfo")));

    return GetAvatarActorFromActorInfo(this);
}
