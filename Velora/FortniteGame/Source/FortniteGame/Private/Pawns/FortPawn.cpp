#include "pch.h"
#include "Pawns/FortPawn.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static auto execMovingEmoteStopped = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawn:MovingEmoteStopped"));

    if (!Function || !AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("bMovingEmote"))))
        return false;

    auto Finish = Scanner::Exec(TEXT("/Script/FortniteGame.FortPawn:MovingEmoteStopped")).Find("mov [*+0x20], *; jmp 0x*");

    if (!Finish.Get() || !Scanner::At(Finish.Find("jmp 0x*").Target()).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked AFortPawn::execMovingEmoteStopped, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&AFortPawn::execMovingEmoteStopped);

    return true;
}();

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

AFortWeapon* AFortPawn::EquipWeaponDefinition(const UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid)
{
    static auto EquipWeaponDefinition = Scanner::Function<AFortWeapon* (*)(AFortPawn*, const UFortWeaponItemDefinition*, const FGuid&)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition")).Get(TEXT("AFortPawn::EquipWeaponDefinition")));

    return EquipWeaponDefinition(this, WeaponData, ItemEntryGuid);
}

bool AFortPawn::EquipBestWeapon()
{
    static auto EquipBestWeapon = Scanner::Exec(TEXT("/Script/FortniteGame.FortPawn:EquipBestWeapon")).Find("call [rax+0x*]").Slot(TEXT("AFortPawn::EquipBestWeapon"));

    return Scanner::Virtual<bool (*)(AFortPawn*)>(this, EquipBestWeapon)(this);
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

bool AFortPawn::GetIsDBNO() const
{
    static auto bIsDBNO = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bIsDBNO")));

    return bIsDBNO->GetPropertyValue_InContainer(this);
}

void AFortPawn::SetIsDBNO(bool bInIsDBNO)
{
    static auto bIsDBNO = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bIsDBNO")));

    bIsDBNO->SetPropertyValue_InContainer(this, bInIsDBNO);
}

void AFortPawn::SetPlayedDying(bool bInPlayedDying)
{
    static auto bPlayedDying = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPlayedDying")));

    bPlayedDying->SetPropertyValue_InContainer(this, bInPlayedDying);
}

void AFortPawn::SetMovingEmote(bool bInMovingEmote)
{
    static auto bMovingEmote = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMovingEmote")));

    bMovingEmote->SetPropertyValue_InContainer(this, bInMovingEmote);
}

void AFortPawn::SetMovingEmoteForwardOnly(bool bInMovingEmoteForwardOnly)
{
    static auto bMovingEmoteForwardOnly = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMovingEmoteForwardOnly")));

    bMovingEmoteForwardOnly->SetPropertyValue_InContainer(this, bInMovingEmoteForwardOnly);
}

void AFortPawn::MovingEmoteStopped()
{
    SetMovingEmote(false);
    SetMovingEmoteForwardOnly(false);
}

void AFortPawn::execMovingEmoteStopped(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    MovingEmoteStopped();
}

FMulticastScriptDelegate& AFortPawn::GetOnWeaponUnEquippedDelegate()
{
    static auto OnWeaponUnEquippedDelegate = GetClass()->FindPropertyByName(FName(TEXT("OnWeaponUnEquippedDelegate")));

    return *OnWeaponUnEquippedDelegate->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

UFortHealthSet* AFortPawn::GetHealthSet() const
{
    static auto HealthSet = GetClass()->FindPropertyByName(FName(TEXT("HealthSet")));

    return *HealthSet->ContainerPtrToValuePtr<UFortHealthSet*>(this);
}

void AFortPawn::LaunchCharacterJump(const FVector& LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent)
{
    static auto LaunchCharacterJump = Scanner::Function<void (*)(AFortPawn*, const FVector&, bool, bool, bool, bool)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPawn:LaunchCharacterJump")).Get(TEXT("AFortPawn::LaunchCharacterJump")));

    LaunchCharacterJump(this, LaunchVelocity, bXYOverride, bZOverride, bIgnoreFallDamage, bPlayFeedbackEvent);
}
