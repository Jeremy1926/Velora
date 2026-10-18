#include "pch.h"
#include "Abilities/FortHealthSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortHealthSet::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortHealthSet"));

    return Class;
}

FGameplayAttribute UFortHealthSet::GetHealthAttribute()
{
    static auto Health = StaticClass()->FindPropertyByName(FName(TEXT("Health")));

    return FGameplayAttribute(Health);
}

FGameplayAttribute UFortHealthSet::GetCurrentShieldAttribute()
{
    static auto CurrentShield = StaticClass()->FindPropertyByName(FName(TEXT("CurrentShield")));

    return FGameplayAttribute(CurrentShield);
}

const FFortGameplayAttributeData& UFortHealthSet::GetHealth() const
{
    static auto Health = GetClass()->FindPropertyByName(FName(TEXT("Health")));

    return *Health->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}

const FFortGameplayAttributeData& UFortHealthSet::GetMaxHealth() const
{
    static auto MaxHealth = GetClass()->FindPropertyByName(FName(TEXT("MaxHealth")));

    return *MaxHealth->ContainerPtrToValuePtr<FFortGameplayAttributeData>(this);
}
