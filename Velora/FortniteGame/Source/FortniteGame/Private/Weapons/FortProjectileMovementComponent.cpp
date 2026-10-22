#include "pch.h"
#include "Weapons/FortProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortProjectileMovementComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortProjectileMovementComponent"));

    return Class;
}

void UFortProjectileMovementComponent::SetVelocity(const FVector& InReplicatedVelocity)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("inc [*+0x*]")
        .Get();

    if (bInlined)
    {
        static auto ReplicatedVelocityCount = Anchor::Offset(TEXT("UFortProjectileMovementComponent::SetVelocity counter"))
            .Exec(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("inc [*+0x*]")
            .Disp();

        static auto ReplicatedVelocity = Anchor::Offset(TEXT("UFortProjectileMovementComponent::ReplicatedVelocity"))
            .Exec(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("movsd [*+0x*], xmm0")
            .Disp();

        static auto OnReplicatedVelocityChanged = Anchor::Function<void (*)(UFortProjectileMovementComponent*)>(TEXT("UFortProjectileMovementComponent::SetVelocity update"))
            .Exec(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("movsd [*+0x*], xmm0")
            .Find("call 0x*")
            .Target()
            .Get();

        if (!ReplicatedVelocityCount || !ReplicatedVelocity)
            return;

        if (auto Owner = GetOwner())
            Owner->FlushNetDormancy();

        ++*((uint8*)this + ReplicatedVelocityCount);
        *(FVector*)((uint8*)this + ReplicatedVelocity) = InReplicatedVelocity;

        OnReplicatedVelocityChanged(this);

        return;
    }

    static auto SetVelocity = Anchor::Function<void (*)(UFortProjectileMovementComponent*, const FVector&)>(TEXT("UFortProjectileMovementComponent::SetVelocity"))
        .Native(TEXT("/Script/FortniteGame.FortProjectileMovementComponent:SetVelocity"))
        .Get();

    SetVelocity(this, InReplicatedVelocity);
}
