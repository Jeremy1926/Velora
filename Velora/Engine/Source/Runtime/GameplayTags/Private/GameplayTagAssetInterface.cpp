#include "pch.h"
#include "GameplayTagAssetInterface.h"
#include "Scanner.h"

void IGameplayTagAssetInterface::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GetOwnedGameplayTags = Anchor::Slot(TEXT("IGameplayTagAssetInterface::GetOwnedGameplayTags"))
        .Exec(TEXT("/Script/GameplayTags.GameplayTagAssetInterface:GetOwnedGameplayTags"))
        .Find("call [*+0x*]")
        .Get();

    Scanner::Virtual<void (*)(const IGameplayTagAssetInterface*, FGameplayTagContainer&)>(this, GetOwnedGameplayTags)(this, TagContainer);
}
