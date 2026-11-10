#include "pch.h"
#include "Components/SkinnedMeshComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

int32 USkinnedMeshComponent::GetBoneIndex(FName BoneName) const
{
    static auto GetRefSkeleton = Anchor::VirtualFunction<uint8* (*)(const UObject*)>(TEXT("USkinnedAsset::GetRefSkeleton"))
        .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov rax, [rcx]; call [rax+0x*]; movzx r8d, bx")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov rcx, rax; mov rdx, [rax]; call [rdx+0x*]; test ebx, ebx; jz 0x*; lea r11, [rax+0x*]")
        .Find("call [rdx+0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("mov rcx, rax; mov rdx, [rax]; call [rdx+0x*]; movzx edx, bx; mov ecx, edx")
        .Find("call [rdx+0x*]")
        .Optional()
        .Get();

    if (GetRefSkeleton)
    {
        static auto RawNameToIndexMap = Anchor::Offset(TEXT("FReferenceSkeleton::RawNameToIndexMap"))
            .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov rdx, [r9+0x*]; movsxd rcx, eax; add rcx, rcx; cmp [rdx+rcx*8], ebx")
            .Or()
            .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov rcx, rax; mov rdx, [rax]; call [rdx+0x*]; test ebx, ebx; jz 0x*; lea r11, [rax+0x*]")
            .Find("lea r11, [rax+0x*]")
            .Or()
            .Exec(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("mov rcx, rax; mov rdx, [rax]; call [rdx+0x*]; movzx edx, bx; mov ecx, edx")
            .Find("lea r11, [rax+0x*]")
            .Disp();

        static auto SkeletalMesh = GetClass()->FindPropertyByName(FName(TEXT("SkeletalMesh")));
        static auto SkinnedAsset = GetClass()->FindPropertyByName(FName(TEXT("SkinnedAsset")));

        auto Mesh = SkeletalMesh ? *SkeletalMesh->ContainerPtrToValuePtr<UObject*>(this) : nullptr;

        if (!Mesh && SkinnedAsset)
            Mesh = *SkinnedAsset->ContainerPtrToValuePtr<UObject*>(this);

        auto RefSkeleton = BoneName != FName(NAME_None) && Mesh && RawNameToIndexMap > 0 ? GetRefSkeleton(Mesh) : nullptr;

        if (!RefSkeleton)
            return INDEX_NONE;

        auto NameToIndexMap = RefSkeleton + RawNameToIndexMap;
        auto AllocationFlags = *(uint32**)(NameToIndexMap + 0x20) ? *(uint32**)(NameToIndexMap + 0x20) : (uint32*)(NameToIndexMap + 0x10);

        for (int32 Index = 0; Index < *(int32*)(NameToIndexMap + 0x8) && Index < *(int32*)(NameToIndexMap + 0x28); Index++)
        {
            auto Element = *(uint8**)NameToIndexMap + Index * 0x10;

            if ((AllocationFlags[Index / 32] & (1u << (Index % 32))) && TGameLayout<FName>::Load(Element) == BoneName)
                return *(int32*)(Element + 0x4);
        }

        return INDEX_NONE;
    }

    static auto GetBoneIndex = Anchor::Function<int32 (*)(const USkinnedMeshComponent*, FName)>(TEXT("USkinnedMeshComponent::GetBoneIndex"))
        .Native(TEXT("/Script/Engine.SkinnedMeshComponent:GetBoneIndex"))
        .Get();

    return GetBoneIndex(this, BoneName);
}
