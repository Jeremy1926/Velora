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

TMulticastDelegate<void, AFortPawn*, AFortWeapon*, AFortWeapon*>& AFortPawn::GetNotifyWeaponChange()
{
    static auto NotifyWeaponChange = Anchor::Data<TMulticastDelegate<void, AFortPawn*, AFortWeapon*, AFortWeapon*>*>(TEXT("AFortPawn::NotifyWeaponChange"))
        .String(TEXT("EquipWeapon - Weapon %s is owned by %s but %s attempted to being equip it."))
        .Find("mov r8, [*+0x*]; mov r9, *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rax, [0x*]")
        .Target()
        .Get();

    return *NotifyWeaponChange;
}

TMulticastDelegate<void, AFortPawn*, float>& AFortPawn::GetNotifyPawnNetCullDistanceChanged()
{
    static auto NotifyPawnNetCullDistanceChanged = Anchor::Data<TMulticastDelegate<void, AFortPawn*, float>*>(TEXT("AFortPawn::NotifyPawnNetCullDistanceChanged"))
        .FieldPattern("F3 0F 11 83 %o F3 0F 10 93 %o 48 8D 0D", AActor::StaticClass() ? AActor::StaticClass()->FindPropertyByName(FName(TEXT("NetCullDistanceSquared"))) : nullptr)
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyPawnNetCullDistanceChanged;
}

AFortWeapon* AFortPawn::GetCurrentWeapon() const
{
    static auto CurrentWeapon = GetClass()->FindPropertyByName(FName(TEXT("CurrentWeapon")));

    return *CurrentWeapon->ContainerPtrToValuePtr<AFortWeapon*>(this);
}

AFortWeapon* AFortPawn::EquipWeaponDefinition(const UFortWeaponItemDefinition* WeaponData, const FGuid& ItemEntryGuid, const FGuid& TrackerGuid)
{
    static auto EquipWeaponDefinition = Anchor::Function<AFortWeapon* (*)(AFortPawn*, const UFortWeaponItemDefinition*, const FGuid&, const FGuid&)>(TEXT("AFortPawn::EquipWeaponDefinition"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Get();

    return EquipWeaponDefinition(this, WeaponData, ItemEntryGuid, TrackerGuid);
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
    static auto SetIsDBNO = Anchor::VirtualFunction<void (*)(AFortPawn*, bool)>(TEXT("AFortPawn::SetIsDBNO"))
        .FieldPattern("44 88 81 %b 41 F6 C0 %m", bIsDBNO)
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPawn"))
        .Optional()
        .Get();

    if (SetIsDBNO)
    {
        SetIsDBNO(this, bInIsDBNO);
        return;
    }

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

void AFortPawn::SetMovingEmoteFollowingOnly(bool bInMovingEmoteFollowingOnly)
{
    static auto bMovingEmoteFollowingOnly = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMovingEmoteFollowingOnly")));

    if (bMovingEmoteFollowingOnly)
        bMovingEmoteFollowingOnly->SetPropertyValue_InContainer(this, bInMovingEmoteFollowingOnly);
}

void AFortPawn::SetMovingEmoteSkipLandingFX(bool bInMovingEmoteSkipLandingFX)
{
    static auto bMovingEmoteSkipLandingFX = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMovingEmoteSkipLandingFX")));

    if (bMovingEmoteSkipLandingFX)
        bMovingEmoteSkipLandingFX->SetPropertyValue_InContainer(this, bInMovingEmoteSkipLandingFX);
}

void AFortPawn::SetEmoteWalkSpeed(float InEmoteWalkSpeed)
{
    static auto EmoteWalkSpeed = GetClass()->FindPropertyByName(FName(TEXT("EmoteWalkSpeed")));

    *EmoteWalkSpeed->ContainerPtrToValuePtr<float>(this) = InEmoteWalkSpeed;
}

void AFortPawn::MovingEmoteStopped()
{
    SetMovingEmote(false);
    SetMovingEmoteForwardOnly(false);
    SetMovingEmoteFollowingOnly(false);
    SetMovingEmoteSkipLandingFX(false);
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
        .Or()
        .If(AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("bMovingEmote"))))
        .StubExec(TEXT("/Script/FortniteGame.FortPawn:MovingEmoteStopped"))
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

UFortItemDefinition* AFortPawn::GetLastReplicatedEmoteExecuted() const
{
    static auto LastReplicatedEmoteExecuted = GetClass()->FindPropertyByName(FName(TEXT("LastReplicatedEmoteExecuted")));

    return LastReplicatedEmoteExecuted ? *LastReplicatedEmoteExecuted->ContainerPtrToValuePtr<UFortItemDefinition*>(this) : nullptr;
}

void AFortPawn::SetLastReplicatedEmoteExecuted(UFortItemDefinition* InLastReplicatedEmoteExecuted)
{
    static auto LastReplicatedEmoteExecuted = GetClass()->FindPropertyByName(FName(TEXT("LastReplicatedEmoteExecuted")));

    if (LastReplicatedEmoteExecuted)
        *LastReplicatedEmoteExecuted->ContainerPtrToValuePtr<UFortItemDefinition*>(this) = InLastReplicatedEmoteExecuted;
}

void AFortPawn::OnRep_LastReplicatedEmoteExecuted(const UFortItemDefinition* PreviousValue)
{
    static auto OnRepFunction = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawn:OnRep_LastReplicatedEmoteExecuted"));
    static auto bTakesPreviousValue = OnRepFunction && OnRepFunction->FindPropertyByName(FName(TEXT("PreviousValue")));

    if (bTakesPreviousValue)
    {
        static auto OnRepWithPreviousValue = Anchor::VirtualFunction<void (*)(AFortPawn*, const UFortItemDefinition*)>(TEXT("AFortPawn::OnRep_LastReplicatedEmoteExecuted previous value"))
            .Exec(TEXT("/Script/FortniteGame.FortPawn:OnRep_LastReplicatedEmoteExecuted"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call [rax+0x*]")
            .Get();

        OnRepWithPreviousValue(this, PreviousValue);

        return;
    }

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
    static auto VirtualSetGravityMultiplier = Anchor::VirtualFunction<void (*)(AFortPawn*, float)>(TEXT("AFortPawn::SetGravityMultiplier"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:SetGravityMultiplier"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call *")
        .Is("call [rax+0x*]")
        .Optional()
        .Get();

    if (VirtualSetGravityMultiplier)
    {
        VirtualSetGravityMultiplier(this, NewGravityVal);

        return;
    }

    static auto SetGravityMultiplier = Anchor::Function<void (*)(AFortPawn*, float)>(TEXT("AFortPawn::SetGravityMultiplier"))
        .Native(TEXT("/Script/FortniteGame.FortPawn:SetGravityMultiplier"))
        .Get();

    SetGravityMultiplier(this, NewGravityVal);
}

void AFortPawn::Suicide()
{
    static auto Suicide = Anchor::VirtualFunction<void (*)(AFortPawn*)>(TEXT("AFortPawn::Suicide"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortPawn"), "mov rax, [rcx]; mov rdx, rcx; jmp [rax+0x*]")
        .Get();

    Suicide(this);
}

TArray<AFortWeapon*>& AFortPawn::GetCurrentWeaponList()
{
    static auto CurrentWeaponList = GetClass()->FindPropertyByName(FName(TEXT("CurrentWeaponList")));

    return *CurrentWeaponList->ContainerPtrToValuePtr<TArray<AFortWeapon*>>(this);
}

void AFortPawn::UpdateCullDistance()
{
    static auto UpdateCullDistance = Anchor::VirtualFunction<void (*)(AFortPawn*)>(TEXT("AFortPawn::UpdateCullDistance"))
        .Exec(TEXT("/Script/FortniteGame.FortPawn:SetPawnVisibility"))
        .Find("call [rax+0x*]")
        .Get(sizeof(void*));

    UpdateCullDistance(this);
}

bool AFortPawn::UpdateSharedReplication()
{
    static auto UpdateSharedReplication = Anchor::VirtualFunction<bool (*)(AFortPawn*)>(TEXT("AFortPawn::UpdateSharedReplication"))
        .Event(TEXT("FastSharedReplication"))
        .Caller()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAIPawn"))
        .Or()
        .Event(TEXT("FastSharedReplication"))
        .Caller()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"))
        .Get();

    return UpdateSharedReplication(this);
}
