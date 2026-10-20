#include "pch.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UProjectileMovementComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.ProjectileMovementComponent"));

    return Class;
}

void UProjectileMovementComponent::SetProjectileGravityScale(float InProjectileGravityScale)
{
    static auto ProjectileGravityScale = StaticClass()->FindPropertyByName(FName(TEXT("ProjectileGravityScale")));

    *ProjectileGravityScale->ContainerPtrToValuePtr<float>(this) = InProjectileGravityScale;
}
