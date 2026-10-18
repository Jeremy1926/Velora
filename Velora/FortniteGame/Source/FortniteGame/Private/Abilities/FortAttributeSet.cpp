#include "pch.h"
#include "Abilities/FortAttributeSet.h"
#include "Scanner.h"

void UFortAttributeSet::SetNumericAttributeBase(const FGameplayAttribute& Attribute, float NewValue)
{
    static auto SetNumericAttributeBase = (void (*)(UFortAttributeSet*, const FGameplayAttribute&, float))Scanner::Native(TEXT("/Script/FortniteGame.FortPawn:SetHealth")).Find("call 0x*", 2).Target(TEXT("UFortAttributeSet::SetNumericAttributeBase"));

    SetNumericAttributeBase(this, Attribute, NewValue);
}
