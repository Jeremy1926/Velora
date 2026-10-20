#include "pch.h"
#include "GameFramework/Character.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

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

void ACharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
    static auto LaunchCharacter = Scanner::Exec(TEXT("/Script/Engine.Character:LaunchCharacter")).Find("call [*+0x*]").Slot(TEXT("ACharacter::LaunchCharacter"));

    Scanner::Virtual<void (*)(ACharacter*, FVector, bool, bool)>(this, LaunchCharacter)(this, LaunchVelocity, bXYOverride, bZOverride);
}
