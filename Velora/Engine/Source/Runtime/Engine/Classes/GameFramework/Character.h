#pragma once

#include "GameFramework/Pawn.h"
#include "Math/Vector.h"

class UCapsuleComponent;
class UCharacterMovementComponent;
class USkeletalMeshComponent;

class ACharacter : public APawn
{
public:
    static UClass* StaticClass();

    USkeletalMeshComponent* GetMesh() const;

    UCharacterMovementComponent* GetCharacterMovement() const;

    UCapsuleComponent* GetCapsuleComponent() const;

    void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

    void ClientCheatWalk();

    void ClientCheatFly();

    void ClientCheatWalk_Implementation();

    void ClientCheatFly_Implementation();
};
