#include "pch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

void UCharacterMovementComponent::SetMovementMode(EMovementMode NewMovementMode, uint8 NewCustomMode)
{
    static auto SetMovementMode = Anchor::VirtualFunction<void (*)(UCharacterMovementComponent*, EMovementMode, uint8)>(TEXT("UCharacterMovementComponent::SetMovementMode"))
        .Exec(TEXT("/Script/Engine.CharacterMovementComponent:SetMovementMode"))
        .Find("call [rax+0x*]")
        .Get();

    SetMovementMode(this, NewMovementMode, NewCustomMode);
}

bool UCharacterMovementComponent::GetCheatFlying() const
{
    static auto bCheatFlying = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCheatFlying")));

    return bCheatFlying->GetPropertyValue_InContainer(this);
}

void UCharacterMovementComponent::SetCheatFlying(bool bInCheatFlying)
{
    static auto bCheatFlying = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCheatFlying")));

    bCheatFlying->SetPropertyValue_InContainer(this, bInCheatFlying);
}
