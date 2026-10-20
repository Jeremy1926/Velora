#pragma once

#include "GameFramework/PlayerState.h"

class APawn;
class UFortAbilitySystemComponent;

class AFortPlayerState : public APlayerState
{
public:
    static UClass* StaticClass();

    void ApplyCharacterCustomization(APawn* Pawn);

    APawn* GetCurrentPawn() const;

    int32& GetWorldPlayerId();

    UFortAbilitySystemComponent*& GetAbilitySystemComponent();
};
