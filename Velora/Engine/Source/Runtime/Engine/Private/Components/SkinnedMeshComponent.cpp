#include "pch.h"
#include "Components/SkinnedMeshComponent.h"
#include "Scanner.h"

int32 USkinnedMeshComponent::GetBoneIndex(FName BoneName) const
{
    static auto GetBoneIndex = Anchor::Function<int32 (*)(const USkinnedMeshComponent*, FName)>(TEXT("USkinnedMeshComponent::GetBoneIndex"))
        .Native(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
        .Get();

    return GetBoneIndex(this, BoneName);
}
