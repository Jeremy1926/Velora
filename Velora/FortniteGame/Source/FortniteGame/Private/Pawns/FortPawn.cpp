#include "pch.h"
#include "Pawns/FortPawn.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortPawn::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawn"));

    return Class;
}

AFortWeapon* AFortPawn::GetCurrentWeapon() const
{
    static auto CurrentWeapon = GetClass()->FindPropertyByName(FName(TEXT("CurrentWeapon")));

    return *CurrentWeapon->ContainerPtrToValuePtr<AFortWeapon*>(this);
}

UAbilitySystemComponent* AFortPawn::GetAbilitySystemComponent() const
{
    static auto AbilitySystemComponent = GetClass()->FindPropertyByName(FName(TEXT("AbilitySystemComponent")));

    return *AbilitySystemComponent->ContainerPtrToValuePtr<UFortAbilitySystemComponent*>(this);
}

void AFortPawn::StopPerformingNativeAction(const FGameplayTag& ActionTag)
{
    static auto StopPerformingNativeAction = []
    {
        auto NotActive = Scanner::String(TEXT("Can't stop native action %s, not active!"));
        auto StopNativeAction = std::format("mov rcx, [rcx+0x*]; test rcx, rcx; jnz {:#x}", (uintptr_t)(NotActive.Get() ? NotActive : Scanner::String(TEXT("Native Action Role(%s) Can't stop %s, not active!"))).Start().Get(TEXT("UFortAbilitySystemComponent::StopPerformingNativeAction")));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPawn"));

        return Scanner::Slot(TEXT("AFortPawn::StopPerformingNativeAction"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Is(StopNativeAction); });
    }();

    Scanner::Virtual<void (*)(AFortPawn*, const FGameplayTag&)>(this, StopPerformingNativeAction)(this, ActionTag);
}

bool& AFortPawn::GetIsDBNO()
{
    static auto bIsDBNO = GetClass()->FindPropertyByName(FName(TEXT("bIsDBNO")));

    return *bIsDBNO->ContainerPtrToValuePtr<bool>(this);
}

bool& AFortPawn::GetPlayedDying()
{
    static auto bPlayedDying = GetClass()->FindPropertyByName(FName(TEXT("bPlayedDying")));

    return *bPlayedDying->ContainerPtrToValuePtr<bool>(this);
}

UFortHealthSet* AFortPawn::GetHealthSet() const
{
    static auto HealthSet = GetClass()->FindPropertyByName(FName(TEXT("HealthSet")));

    return *HealthSet->ContainerPtrToValuePtr<UFortHealthSet*>(this);
}
