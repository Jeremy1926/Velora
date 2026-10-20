#include "pch.h"
#include "GameFramework/Character.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

USkeletalMeshComponent* ACharacter::GetMesh() const
{
    static auto Mesh = GetClass()->FindPropertyByName(FName(TEXT("Mesh")));

    return *Mesh->ContainerPtrToValuePtr<USkeletalMeshComponent*>(this);
}

UCapsuleComponent* ACharacter::GetCapsuleComponent() const
{
    static auto CapsuleComponent = GetClass()->FindPropertyByName(FName(TEXT("CapsuleComponent")));

    return *CapsuleComponent->ContainerPtrToValuePtr<UCapsuleComponent*>(this);
}
