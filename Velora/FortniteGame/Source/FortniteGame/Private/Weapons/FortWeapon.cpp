#include "pch.h"
#include "Weapons/FortWeapon.h"
#include "Weapons/FortWeaponModItemDefinition.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
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

UFortWeaponItemDefinition* AFortWeapon::GetWeaponData() const
{
    static auto WeaponData = StaticClass()->FindPropertyByName(FName(TEXT("WeaponData")));

    return *WeaponData->ContainerPtrToValuePtr<UFortWeaponItemDefinition*>(this);
}

void AFortWeapon::PlayWeaponFireFX(bool bPersistentFire)
{
    static auto PlayWeaponFireFX = Anchor::VirtualFunction<void (*)(AFortWeapon*, bool)>(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .Event(TEXT("OnPlayWeaponFireFX"))
        .Log(TEXT("AFortWeapon::OnPlayWeaponFireFX"))
        .Caller()
        .Log(TEXT("AFortWeapon::PlayWeaponFireFX"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Or()
        .String(TEXT("STAT_FortWeapon_PlayWeaponFireAudio"))
        .Start()
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
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortWeapon:TryReloadAmmoFromAbility"))
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeapon:K2_ConsumeAmmo"))
        .Find("setnz *")
        .Find("neg edx")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeapon:ModAmmo"))
        .Find("setnz *")
        .Find("call [rax+0x*]")
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

    FlushPendingUnequipRemovals();

    auto Pawn = Cast<AFortPawn>(GetInstigator());

    if (Pawn && Pawn->GetCurrentWeapon() == this)
    {
        GetPendingUnequipRemovals().AddUnique(FWeakObjectPtr(this));
        return;
    }

    AttemptRemoveGrantedAbilities(EFortWeaponAbilityRemovalReason::Unequipped);
}

TArray<FWeakObjectPtr>& AFortWeapon::GetPendingUnequipRemovals()
{
    static TArray<FWeakObjectPtr> PendingUnequipRemovals;

    return PendingUnequipRemovals;
}

void AFortWeapon::FlushPendingUnequipRemovals()
{
    auto& Pending = GetPendingUnequipRemovals();

    for (int32 i = 0; i < Pending.Num(); i++)
    {
        auto Weapon = (AFortWeapon*)Pending[i].Get(true);
        auto Pawn = Weapon ? Cast<AFortPawn>(Weapon->GetInstigator()) : nullptr;

        if (Weapon && !(Pawn && Pawn->GetCurrentWeapon() == Weapon))
            Weapon->AttemptRemoveGrantedAbilities(EFortWeaponAbilityRemovalReason::Unequipped);
    }

    Pending.Empty();
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

UFortGameplayAbility* AFortWeapon::GetActiveAbility() const
{
    static auto ActiveAbility = StaticClass()->FindPropertyByName(FName(TEXT("ActiveAbility")));

    return ActiveAbility ? *ActiveAbility->ContainerPtrToValuePtr<UFortGameplayAbility*>(this) : nullptr;
}

FGameplayAbilitySpecHandle AFortWeapon::GetPrimaryAbilitySpecHandle() const
{
    static auto PrimaryAbilitySpecHandle = StaticClass()->FindPropertyByName(FName(TEXT("PrimaryAbilitySpecHandle")));

    return PrimaryAbilitySpecHandle ? *PrimaryAbilitySpecHandle->ContainerPtrToValuePtr<FGameplayAbilitySpecHandle>(this) : FGameplayAbilitySpecHandle();
}

bool AFortWeapon::GetInitializedWeaponItem() const
{
    static auto bInitializedWeaponItem = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bInitializedWeaponItem")));

    return bInitializedWeaponItem && bInitializedWeaponItem->GetPropertyValue_InContainer(this);
}

void AFortWeapon::SetAmmoCount(int32 NewAmmoCount)
{
    static auto AmmoCount = StaticClass()->FindPropertyByName(FName(TEXT("AmmoCount")));

    if (*AmmoCount->ContainerPtrToValuePtr<int32>(this) != NewAmmoCount)
        *AmmoCount->ContainerPtrToValuePtr<int32>(this) = NewAmmoCount;
}

void AFortWeapon::InitializeFromItem(UFortWorldItem* Item)
{
    DETOUR(AFortWeapon, InitializeFromItem, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rdx, rax; mov rcx, *; call [r8+0x*]")
        .Find("call [r8+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Is("test rdx, rdx; jz *")
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Find("call 0x*; mov rbx, rax; test rax, rax; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, rax; mov rcx, *; call [r8+0x*]")
        .Find("call [r8+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Is("test rdx, rdx; jz *")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Find("setnz *; add r*, r*")
        .Find("cmp byte ptr [*+0x*], 0x3")
        .Smallest(TEXT("STAT_AFortPawn_EquipWeaponDefinition"))
        .Start()
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rax, [*]; mov rdx, *; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortWeapon"))
        .Is("test rdx, rdx; jz *")
        .Optional()
        .Inline());

    Original(Item);

    if (!Item || !HasAuthority())
        return;

    if (!GetInitializedWeaponItem())
        SetAmmoCount(Item->GetItemEntry().GetLoadedAmmo());

    static auto WeaponModSlots = FFortItemEntry::StaticStruct()->FindPropertyByName(FName(TEXT("WeaponModSlots")));

    if (!WeaponModSlots)
        return;

    for (auto& WeaponModSlot : Item->GetItemEntry().GetWeaponModSlots())
        AddWeaponModSlot(WeaponModSlot);
}

UFortWorldItem* AFortWeapon::GetInventoryItem() const
{
    auto Pawn = Cast<AFortPawn>(GetOwner());
    auto PlayerController = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WorldInventory)
        return nullptr;

    for (auto Item : WorldInventory->GetInventory().GetItemInstances())
    {
        if (Item && Item->GetItemGuid() == ((AFortWeapon*)this)->GetItemEntryGuid())
            return Item;
    }

    return nullptr;
}

const TArray<FFortWeaponModSlot>& AFortWeapon::GetWeaponModSlots() const
{
    static auto WeaponModSlots = StaticClass()->FindPropertyByName(FName(TEXT("WeaponModSlots")));

    return *WeaponModSlots->ContainerPtrToValuePtr<TArray<FFortWeaponModSlot>>(this);
}

bool AFortWeapon::CanAddWeaponMod(const UFortWeaponModItemDefinition* WeaponMod) const
{
    static auto CanAddWeaponMod = Anchor::Function<bool (*)(const AFortWeapon*, const UFortWeaponModItemDefinition*)>(TEXT("AFortWeapon::CanAddWeaponMod"))
        .Exec(TEXT("/Script/WeaponModsCodeRuntime.FortWeaponModStationComponent:ServerPurchaseWeaponModForWeapon"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/WeaponModsCodeRuntime.Default__FortWeaponModStationComponent"))
        .Find("call 0x*; test al, al; jz 0x*", 1)
        .Call()
        .Get();

    return CanAddWeaponMod(this, WeaponMod);
}

bool AFortWeapon::AddWeaponModSlot(const FFortWeaponModSlot& WeaponModSlot)
{
    static auto AddWeaponModSlot = Anchor::Function<bool (*)(AFortWeapon*, const FFortWeaponModSlot&)>(TEXT("AFortWeapon::AddWeaponModSlot"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:TryAddPreviewWeaponMod"))
        .Find("mov byte ptr [rbp-0x*], 0x1; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:TryAddPreviewWeaponMod"))
        .Find("lea rdx, [rbp-0x*]; mov [rbp-0x*], *; mov rcx, *; mov [rbp-0x*], *; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Get();

    return AddWeaponModSlot(this, WeaponModSlot);
}

bool AFortWeapon::RemoveWeaponModSlot(int32 SlotIndex)
{
    static auto RemoveWeaponModSlot = Anchor::Function<bool (*)(AFortWeapon*, int32)>(TEXT("AFortWeapon::RemoveWeaponModSlot"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:TryRemovePreviewWeaponMod"))
        .Find("jns 0x*; jmp 0x*; call 0x*; mov *, al")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:TryRemovePreviewWeaponMod"))
        .Find("setz al; test al, al; jz 0x*; call 0x*; mov *, al")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponModFunctionLibrary:TryRemovePreviewWeaponMod"))
        .Find("setz al; test al, al; jz 0x*; mov rcx, r8; call 0x*; mov *, al")
        .Find("call 0x*")
        .Target()
        .Get();

    return RemoveWeaponModSlot(this, SlotIndex);
}

void AFortWeapon::UpdateCurrentWeaponModSlotsOnWorldItem()
{
    if (auto Item = GetInventoryItem())
        Item->GetItemEntry().SetWeaponModSlots(GetWeaponModSlots());
}
