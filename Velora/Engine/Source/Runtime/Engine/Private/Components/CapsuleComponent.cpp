#include "pch.h"
#include "Components/CapsuleComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

float UCapsuleComponent::GetUnscaledCapsuleHalfHeight() const
{
    static auto CapsuleHalfHeight = GetClass()->FindPropertyByName(FName(TEXT("CapsuleHalfHeight")));

    return *CapsuleHalfHeight->ContainerPtrToValuePtr<float>(this);
}

float UCapsuleComponent::GetUnscaledCapsuleRadius() const
{
    static auto CapsuleRadius = GetClass()->FindPropertyByName(FName(TEXT("CapsuleRadius")));

    return *CapsuleRadius->ContainerPtrToValuePtr<float>(this);
}
