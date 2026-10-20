#include "pch.h"
#include "GameplayTagAssetInterface.h"
#include "Scanner.h"

void IGameplayTagAssetInterface::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GetOwnedGameplayTags = Anchor::VirtualFunction<void (*)(const IGameplayTagAssetInterface*, FGameplayTagContainer&)>(TEXT("IGameplayTagAssetInterface::GetOwnedGameplayTags"))
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags"))
        .Find("call [*+0x*]")
        .Get();

    GetOwnedGameplayTags(this, TagContainer);
}
