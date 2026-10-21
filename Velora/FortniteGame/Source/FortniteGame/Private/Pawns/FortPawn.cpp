#include "pch.h"
#include "Pawns/FortPawn.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

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
    static auto EquipWeaponDefinition = Anchor::Function<AFortWeapon* (*)(AFortPawn*, const UFortWeaponItemDefinition*, const FGuid&)>(TEXT("AFortPawn::EquipWeaponDefinition"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Get();

    return EquipWeaponDefinition(this, WeaponData, ItemEntryGuid);
}

bool AFortPawn::EquipBestWeapon()
{
    static auto EquipBestWeapon = Anchor::VirtualFunction<bool (*)(AFortPawn*)>(TEXT("AFortPawn::EquipBestWeapon"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:EquipBestWeapon"))
        .Find("call [rax+0x*]")
        .Get();

    return EquipBestWeapon(this);
}

UAbilitySystemComponent* AFortPawn::GetAbilitySystemComponent() const
{
    static auto AbilitySystemComponent = GetClass()->FindPropertyByName(FName(TEXT("AbilitySystemComponent")));

    return *AbilitySystemComponent->ContainerPtrToValuePtr<UFortAbilitySystemComponent*>(this);
}

void AFortPawn::StopPerformingNativeAction(const FGameplayTag& ActionTag)
{
    static auto StopPerformingNativeAction = Anchor::VirtualFunction<void (*)(AFortPawn*, const FGameplayTag&)>(TEXT("AFortPawn::StopPerformingNativeAction"))
        .Strings(TEXT("Can't stop native action %s, not active!"), TEXT("Native Action Role(%s) Can't stop %s, not active!"))
        .Start()
        .Log(TEXT("UFortAbilitySystemComponent::StopPerformingNativeAction"))
        .VirtualIs(TEXT("/Script/FortniteGame.Default__FortPawn"), "mov rcx, [rcx+0x*]; test rcx, rcx; jnz %t")
        .Get();

    StopPerformingNativeAction(this, ActionTag);
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

bool AFortPawn::GetWasDBNOOnDeath() const
{
    static auto bWasDBNOOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bWasDBNOOnDeath")));

    return bWasDBNOOnDeath->GetPropertyValue_InContainer(this);
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
    DETOUR(AFortPawn, execMovingEmoteStopped, Anchor::Hook()
        .If(AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("bMovingEmote"))))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:MovingEmoteStopped"))
        .Find("mov [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPawn:MovingEmoteStopped")));

    Stack.Code += !!Stack.Code;

    MovingEmoteStopped();
}

FMulticastScriptDelegate& AFortPawn::GetOnWeaponUnEquippedDelegate()
{
    static auto OnWeaponUnEquippedDelegate = GetClass()->FindPropertyByName(FName(TEXT("OnWeaponUnEquippedDelegate")));

    return *OnWeaponUnEquippedDelegate->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

void AFortPawn::SetLastReplicatedEmoteExecuted(UFortItemDefinition* InLastReplicatedEmoteExecuted)
{
    static auto LastReplicatedEmoteExecuted = GetClass()->FindPropertyByName(FName(TEXT("LastReplicatedEmoteExecuted")));

    if (LastReplicatedEmoteExecuted)
        *LastReplicatedEmoteExecuted->ContainerPtrToValuePtr<UFortItemDefinition*>(this) = InLastReplicatedEmoteExecuted;
}

void AFortPawn::OnRep_LastReplicatedEmoteExecuted()
{
    static auto OnRep_LastReplicatedEmoteExecuted = Anchor::Optional<void (*)(AFortPawn*)>(TEXT("AFortPawn::OnRep_LastReplicatedEmoteExecuted"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:OnRep_LastReplicatedEmoteExecuted"))
        .Find("mov [*+0x20], *")
        .Is("mov [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Get();

    if (OnRep_LastReplicatedEmoteExecuted)
        return OnRep_LastReplicatedEmoteExecuted(this);

    static auto VirtualOnRep_LastReplicatedEmoteExecuted = Anchor::VirtualFunction<void (*)(AFortPawn*)>(TEXT("AFortPawn::OnRep_LastReplicatedEmoteExecuted"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:OnRep_LastReplicatedEmoteExecuted"))
        .Find("mov [*+0x20], *")
        .Is("mov [*+0x20], *; mov rax, [rcx]; jmp [rax+0x*]")
        .Optional()
        .Find("jmp [rax+0x*]")
        .Get();

    VirtualOnRep_LastReplicatedEmoteExecuted(this);
}

UFortHealthSet* AFortPawn::GetHealthSet() const
{
    static auto HealthSet = GetClass()->FindPropertyByName(FName(TEXT("HealthSet")));

    return *HealthSet->ContainerPtrToValuePtr<UFortHealthSet*>(this);
}

void AFortPawn::LaunchCharacterJump(const FVector& LaunchVelocity, bool bXYOverride, bool bZOverride, bool bIgnoreFallDamage, bool bPlayFeedbackEvent)
{
    static auto LaunchCharacterJump = Anchor::Optional<void (*)(AFortPawn*, const FVector&, bool, bool, bool, bool)>(TEXT("AFortPawn::LaunchCharacterJump"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:LaunchCharacterJump"))
        .Get();

    if (LaunchCharacterJump)
        return LaunchCharacterJump(this, LaunchVelocity, bXYOverride, bZOverride, bIgnoreFallDamage, bPlayFeedbackEvent);

    static auto VirtualLaunchCharacterJump = Anchor::VirtualFunction<void (*)(AFortPawn*, const FVector&, bool, bool, bool, bool)>(TEXT("AFortPawn::LaunchCharacterJump"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:LaunchCharacterJump"))
        .Find("mov [*+0x20], *")
        .Find("call [*+0x*]")
        .Get();

    VirtualLaunchCharacterJump(this, LaunchVelocity, bXYOverride, bZOverride, bIgnoreFallDamage, bPlayFeedbackEvent);
}

void AFortPawn::SetGravityMultiplier(float NewGravityVal)
{
    static auto SetGravityMultiplier = Anchor::Function<void (*)(AFortPawn*, float)>(TEXT("AFortPawn::SetGravityMultiplier"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:SetGravityMultiplier"))
        .Get();

    SetGravityMultiplier(this, NewGravityVal);
}
