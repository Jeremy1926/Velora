#include "pch.h"
#include "Weapons/FortWeapon.h"
#include "AI/FortAthenaAIBotController.h"
#include "Player/FortPlayerController.h"
#include "Items/FortQuickBars.h"
#include "Pawns/FortPawn.h"
#include "Abilities/FortAbilitySet.h"
#include "FortGameModeZone.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortWeapon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeapon"));

    return Class;
}

EFortWeaponType AFortWeapon::FindWeaponTypeForStats(const UFortItemDefinition* ItemDefinition)
{
    static auto FindWeaponTypeForStats = Anchor::Function<EFortWeaponType (*)(const UFortItemDefinition*)>(TEXT("AFortWeapon::FindWeaponTypeForStats"))
        .FieldPattern("41 C7 00 FF FF FF FF ? 8B ? 48 83 B9 %o 00", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("Inventory"))) : nullptr)
        .Find("cmp [*+0x*], *; jnz *; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]; cmp al, 0x*; jnz *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .FieldPattern("41 C7 00 FF FF FF FF 48 8B 89 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("CachedBotManager"))) : nullptr)
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]; cmp al, 0x*; jnz *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return FindWeaponTypeForStats(ItemDefinition);
}

void AFortWeapon::PlayWeaponFireFX(bool bPersistentFire)
{
    static auto PlayWeaponFireFX = Anchor::VirtualFunction<void (*)(AFortWeapon*, bool)>(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .Event(TEXT("OnPlayWeaponFireFX"))
        .Log(TEXT("AFortWeapon::OnPlayWeaponFireFX"))
        .Caller()
        .Log(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Get();

    PlayWeaponFireFX(this, bPersistentFire);
}

bool AFortWeapon::CheckPendingDestroyedDueToDurability()
{
    DETOUR(AFortWeapon, CheckPendingDestroyedDueToDurability, Anchor::Hook()
        .FieldPattern("F6 ? %b %m", (UBoolProperty*)AFortWeapon::StaticClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount"))))
        .Start()
        .Inline());

    auto Instigator = GetInstigator();
    auto PlayerController = Instigator ? Cast<AFortPlayerController>(Instigator->GetController()) : nullptr;
    auto QuickBars = PlayerController ? PlayerController->GetQuickBars() : nullptr;

    if (QuickBars && GetPendingDestroyDueToDurabilityOrStackCount())
    {
        for (auto QuickBarType : { EFortQuickBars::Primary, EFortQuickBars::Secondary })
        {
            auto& QuickBar = QuickBars->GetQuickBar(QuickBarType);
            auto FocusedSlot = QuickBar.CurrentFocusedSlot;

            if (FocusedSlot >= 0 && FocusedSlot < QuickBar.Slots.Num() && !QuickBar.Slots[FocusedSlot].Items.Num() && QuickBars->TrySetFocused(QuickBarType, QuickBar.PreviousFocusedSlot, false) == -1)
                QuickBars->TrySetFocused(QuickBarType, -2, false);
        }
    }

    return Original();
}

bool AFortWeapon::TryReloadAmmo()
{
    static auto TryReloadAmmo = Anchor::VirtualFunction<bool (*)(AFortWeapon*)>(TEXT("AFortWeapon::TryReloadAmmo"))
        .String(TEXT("Maximum reload count at respawn reached on %s"))
        .Start()
        .Find("call [rax+0x*]")
        .Get();

    return TryReloadAmmo(this);
}

bool AFortWeapon::TryReloadAmmoFromAbility()
{
    return TryReloadAmmo();
}

void AFortWeapon::execTryReloadAmmoFromAbility(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortWeapon, execTryReloadAmmoFromAbility, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortWeapon:TryReloadAmmoFromAbility"))
        .Is("xor al, al; ret")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortWeapon:TryReloadAmmoFromAbility")));

    Stack.Code += !!Stack.Code;

    *(bool*)Z_Param__Result = TryReloadAmmoFromAbility();
}

void AFortWeapon::ModAmmo(int32 AmountToUse)
{
    static auto ModAmmo = Anchor::VirtualFunction<void (*)(AFortWeapon*, int32)>(TEXT("AFortWeapon::ModAmmo"))
        .Native(TEXT("/Script/FortniteGame.FortWeapon:K2_ConsumeAmmo"))
        .Is("mov rax, [rcx]; neg edx; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Get();

    ModAmmo(this, AmountToUse);
}

void AFortWeapon::OnUnEquip()
{
    DETOUR(AFortWeapon, OnUnEquip, Anchor::Hook()
        .If(AFortWeapon::StaticClass()->FindPropertyByName(FName(TEXT("GrantedAbilityRemovalPolicy"))))
        .Event(TEXT("K2_OnUnEquip"))
        .Caller()
        .Inline());

    Original();

    AttemptRemoveGrantedAbilities(EFortWeaponAbilityRemovalReason::Unequipped);
}

void AFortWeapon::AttemptRemoveGrantedAbilities(EFortWeaponAbilityRemovalReason Reason)
{
    static auto GrantedAbilityRemovalPolicy = StaticClass()->FindPropertyByName(FName(TEXT("GrantedAbilityRemovalPolicy")));

    auto RemovalPolicy = *GrantedAbilityRemovalPolicy->ContainerPtrToValuePtr<EFortWeaponAbilityRemovalPolicy>(this, (int32)Reason);
    auto GameMode = Cast<AFortGameModeZone>(GetWorld()->GetAuthGameMode());

    if (RemovalPolicy == EFortWeaponAbilityRemovalPolicy::Remove || (RemovalPolicy == EFortWeaponAbilityRemovalPolicy::GameDefault && GameMode && GameMode->ShouldWeaponRemoveGrantedAbilities(this, Reason)))
        RemoveGrantedAbilities();
}

void AFortWeapon::RemoveGrantedAbilities()
{
    static auto WeaponData = StaticClass()->FindPropertyByName(FName(TEXT("WeaponData")));
    static UProperty* const AbilitySpecHandles[] =
    {
        StaticClass()->FindPropertyByName(FName(TEXT("PrimaryAbilitySpecHandle"))),
        StaticClass()->FindPropertyByName(FName(TEXT("SecondaryAbilitySpecHandle"))),
        StaticClass()->FindPropertyByName(FName(TEXT("ReloadAbilitySpecHandle"))),
        StaticClass()->FindPropertyByName(FName(TEXT("ImpactAbilitySpecHandle"))),
    };
    static auto EquippedAbilityHandles = StaticClass()->FindPropertyByName(FName(TEXT("EquippedAbilityHandles")));
    static auto EquippedAbilitySetHandles = StaticClass()->FindPropertyByName(FName(TEXT("EquippedAbilitySetHandles")));

    auto Pawn = Cast<AFortPawn>(GetInstigator());
    auto AbilitySystemComponent = Pawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn) : nullptr;

    if (!AbilitySystemComponent || !*WeaponData->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this) || !HasAuthority())
        return;

    for (auto AbilitySpecHandle : AbilitySpecHandles)
    {
        auto& Handle = *AbilitySpecHandle->ContainerPtrToValuePtr<FGameplayAbilitySpecHandle>(this);

        if (!Handle.IsValid())
            continue;

        AbilitySystemComponent->ClearAbility(Handle);
        Handle = FGameplayAbilitySpecHandle();
    }

    auto& AbilityHandles = *EquippedAbilityHandles->ContainerPtrToValuePtr<TArray<FGameplayAbilitySpecHandle>>(this);

    for (auto& Handle : AbilityHandles)
    {
        if (Handle.IsValid())
            AbilitySystemComponent->ClearAbility(Handle);
    }

    AbilityHandles.Empty();

    auto& AbilitySetHandles = *EquippedAbilitySetHandles->ContainerPtrToValuePtr<TRuntimeSizedArray<FFortAbilitySetHandle>>(this);

    for (auto& AbilitySetHandle : AbilitySetHandles)
    {
        if (AbilitySetHandle.IsValid())
            UFortAbilitySet::UnequipAbilitySet(AbilitySetHandle);
    }

    AbilitySetHandles.RemoveAt(0, AbilitySetHandles.Num());
}

bool AFortWeapon::GetPendingDestroyDueToDurabilityOrStackCount() const
{
    static auto bPendingDestroyDueToDurabilityOrStackCount = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPendingDestroyDueToDurabilityOrStackCount")));

    return bPendingDestroyDueToDurabilityOrStackCount->GetPropertyValue_InContainer(this);
}

FGuid& AFortWeapon::GetItemEntryGuid()
{
    static auto ItemEntryGuid = GetClass()->FindPropertyByName(FName(TEXT("ItemEntryGuid")));

    return *ItemEntryGuid->ContainerPtrToValuePtr<FGuid>(this);
}

float& AFortWeapon::GetLastFireTimeVerified()
{
    static auto LastFireTimeVerified = StaticClass()->FindPropertyByName(FName(TEXT("LastFireTimeVerified")));

    return *LastFireTimeVerified->ContainerPtrToValuePtr<float>(this);
}
