#include "pch.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

void AActor::SetActorEnableCollision(bool bNewActorEnableCollision)
{
    static auto SetActorEnableCollision = Anchor::Function<void (*)(AActor*, bool)>(TEXT("AActor::SetActorEnableCollision"))
        .Native(TEXT("/Script/Engine.Actor:SetActorEnableCollision"))
        .Get();

    SetActorEnableCollision(this, bNewActorEnableCollision);
}

ENetRole AActor::GetRemoteRole() const
{
    static auto RemoteRole = GetClass()->FindPropertyByName(FName(TEXT("RemoteRole")));

    return (ENetRole)*RemoteRole->ContainerPtrToValuePtr<uint8>(this);
}

ENetRole AActor::GetLocalRole() const
{
    static auto Role = GetClass()->FindPropertyByName(FName(TEXT("Role")));

    return (ENetRole)*Role->ContainerPtrToValuePtr<uint8>(this);
}
