#pragma once

#include "GameFramework/Pawn.h"
#include "Math/Vector.h"

class UCapsuleComponent;
class USkeletalMeshComponent;

class ACharacter : public APawn
{
public:
    USkeletalMeshComponent* GetMesh() const;

    UCapsuleComponent* GetCapsuleComponent() const;

    void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);
};
