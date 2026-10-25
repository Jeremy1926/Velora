#include "pch.h"
#include "GameFramework/Actor.h"
#include "Scanner.h"

void AActor::SetActorEnableCollision(bool bNewActorEnableCollision)
{
    static auto SetActorEnableCollision = Anchor::Function<void (*)(AActor*, bool)>(TEXT("AActor::SetActorEnableCollision"))
        .Native(TEXT("/Script/Engine.Actor:SetActorEnableCollision"))
        .Get();

    SetActorEnableCollision(this, bNewActorEnableCollision);
}
