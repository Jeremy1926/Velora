#pragma once

#include "GameFramework/Pawn.h"

class UCapsuleComponent;
class USkeletalMeshComponent;

class ACharacter : public APawn
{
public:
    USkeletalMeshComponent* GetMesh() const;

    UCapsuleComponent* GetCapsuleComponent() const;
};
