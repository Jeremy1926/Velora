#include "pch.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* ACharacter::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Character"));

    return Class;
}

USkeletalMeshComponent* ACharacter::GetMesh() const
{
    static auto Mesh = GetClass()->FindPropertyByName(FName(TEXT("Mesh")));

    return *Mesh->ContainerPtrToValuePtr<USkeletalMeshComponent*>(this);
}

UCharacterMovementComponent* ACharacter::GetCharacterMovement() const
{
    static auto CharacterMovement = GetClass()->FindPropertyByName(FName(TEXT("CharacterMovement")));

    return *CharacterMovement->ContainerPtrToValuePtr<UCharacterMovementComponent*>(this);
}

UCapsuleComponent* ACharacter::GetCapsuleComponent() const
{
    static auto CapsuleComponent = GetClass()->FindPropertyByName(FName(TEXT("CapsuleComponent")));

    return *CapsuleComponent->ContainerPtrToValuePtr<UCapsuleComponent*>(this);
}

void ACharacter::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
    static auto LaunchCharacter = Anchor::VirtualFunction<void (*)(ACharacter*, TGameValue<FVector>, bool, bool)>(TEXT("ACharacter::LaunchCharacter"))
        .Exec(TEXT("/Script/Engine.Character:LaunchCharacter"))
        .Find("call [*+0x*]")
        .Get();

    LaunchCharacter(this, LaunchVelocity, bXYOverride, bZOverride);
}

void ACharacter::ClientCheatWalk()
{
    static auto ClientCheatWalk = Anchor::Function<void (*)(ACharacter*)>(TEXT("ACharacter::ClientCheatWalk"))
        .Event(TEXT("ClientCheatWalk"))
        .Get();

    ClientCheatWalk(this);
}

void ACharacter::ClientCheatFly()
{
    static auto ClientCheatFly = Anchor::Function<void (*)(ACharacter*)>(TEXT("ACharacter::ClientCheatFly"))
        .Event(TEXT("ClientCheatFly"))
        .Get();

    ClientCheatFly(this);
}

void ACharacter::ClientCheatWalk_Implementation()
{
    DETOUR(ACharacter, ClientCheatWalk_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/Engine.Character:ClientCheatWalk"))
        .FindEither("jmp [rax+*]", "call [rax+*]")
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena")));

    SetActorEnableCollision(true);

    if (auto CharacterMovement = GetCharacterMovement())
    {
        CharacterMovement->SetCheatFlying(false);
        CharacterMovement->SetMovementMode(MOVE_Falling);
    }
}

void ACharacter::ClientCheatFly_Implementation()
{
    DETOUR(ACharacter, ClientCheatFly_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/Engine.Character:ClientCheatFly"))
        .FindEither("jmp [rax+*]", "call [rax+*]")
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena")));

    SetActorEnableCollision(true);

    if (auto CharacterMovement = GetCharacterMovement())
    {
        CharacterMovement->SetCheatFlying(true);
        CharacterMovement->SetMovementMode(MOVE_Flying);
    }
}
