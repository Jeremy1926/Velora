#include "pch.h"
#include "Abilities/FortGameplayAbility.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Items/FortInventory.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Pawns/FortPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortGameplayAbility::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbility"));

    return Class;
}

void UFortGameplayAbility::EquipAbilityWeapon(UFortWeaponItemDefinition* WeaponDefinition, int32 ItemLevel)
{
    auto Pawn = Cast<AFortPawn>(GetAvatarActorFromActorInfo());
    auto PlayerController = Pawn ? Cast<AFortPlayerControllerAthena>(Pawn->GetController()) : nullptr;
    auto WorldInventory = PlayerController ? PlayerController->GetWorldInventory() : nullptr;

    if (!WeaponDefinition || !WorldInventory || !Pawn->HasAuthority())
        return;

    auto& Inventory = WorldInventory->GetInventory();
    auto ItemEntry = new(Inventory.GetReplicatedEntries()) FFortItemEntry(WeaponDefinition, 1, ItemLevel);
    const FGuid ItemGuid = ItemEntry->GetItemGuid();

    if (auto WeaponStats = WeaponDefinition->GetWeaponStats<FFortBaseWeaponStats>())
        ItemEntry->SetLoadedAmmo(WeaponDefinition->GetBaseClipSizeWithMods(nullptr, ItemEntry, WeaponStats, ItemEntry->GetLevel()));

    ItemEntry->GetParentInventory() = WorldInventory;
    ItemEntry->GetIsReplicatedCopy() = true;
    WorldInventory->SetItemRequiresUpdate(*ItemEntry);
    ItemEntry->GetIsDirty() = true;

    WorldInventory->HandleInventoryLocalUpdate();

    FGuid TrackerGuid;

    for (auto Item : Inventory.GetItemInstances())
    {
        if (Item->GetItemGuid() == ItemGuid)
        {
            GetAbilityWeaponItem() = Item;
            TrackerGuid = Item->GetTrackerGuid();
        }
    }

    GetAbilityWeapon() = Pawn->EquipWeaponDefinition(WeaponDefinition, ItemGuid, TrackerGuid);
}

void UFortGameplayAbility::execEquipAbilityWeapon(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGameplayAbility, execEquipAbilityWeapon, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerController:ClientEndAbilityWeapon")))
        .Native(TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon"))
        .Is("ret*")
        .Optional()
        .If([]
        {
            auto AbilityWeaponItem = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponItem"))
                .Event(TEXT("ClientEndAbilityWeapon"))
                .CallSite()
                .Last("mov dword ptr [*+0x*], 0xffffffff")
                .Disp();

            return AbilityWeaponItem > 0 && Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponActor"))
                .Event(TEXT("ClientEndAbilityWeapon"))
                .CallSite()
                .FirstOf("add *, 0x*", "lea *, [*+0x*]")
                .Get() > 0;
        })
        .InExec(TEXT("/Script/FortniteGame.FortGameplayAbility:EquipAbilityWeapon")));

    UFortWeaponItemDefinition* WeaponDefinition = nullptr;
    int32 ItemLevel = 0;

    Stack.StepCompiledIn(&WeaponDefinition);
    Stack.StepCompiledIn(&ItemLevel);
    Stack.Code += !!Stack.Code;

    EquipAbilityWeapon(WeaponDefinition, ItemLevel);
}

TWeakObjectPtr<UFortWorldItem>& UFortGameplayAbility::GetAbilityWeaponItem()
{
    static auto AbilityWeaponItem = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponItem"))
        .Event(TEXT("ClientEndAbilityWeapon"))
        .CallSite()
        .Last("mov dword ptr [*+0x*], 0xffffffff")
        .Disp();

    return *(TWeakObjectPtr<UFortWorldItem>*)((uint8_t*)this + AbilityWeaponItem);
}

TWeakObjectPtr<AFortWeapon>& UFortGameplayAbility::GetAbilityWeapon()
{
    static auto AbilityWeapon = Anchor::Offset(TEXT("UFortGameplayAbility::AbilityWeaponActor"))
        .Event(TEXT("ClientEndAbilityWeapon"))
        .CallSite()
        .FirstOf("add *, 0x*", "lea *, [*+0x*]")
        .Get();

    return *(TWeakObjectPtr<AFortWeapon>*)((uint8_t*)this + AbilityWeapon);
}

TArray<FActiveGameplayEffectHandle> UFortGameplayAbility::ApplyGameplayEffectContainer(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag, int32 GameplayEffectLevel)
{
    static auto ApplyGameplayEffectContainer = Anchor::VirtualFunction<TArray<FActiveGameplayEffectHandle>* (*)(UFortGameplayAbility*, TArray<FActiveGameplayEffectHandle>*, const FGameplayAbilityTargetDataHandle*, FGameplayTag, int32)>(TEXT("UFortGameplayAbility::ApplyGameplayEffectContainer"))
        .Exec(TEXT("/Script/FortniteGame.FortGameplayAbility:ApplyGameplayEffectContainer"))
        .Find("add [*+0x20], *")
        .Find("mov rsi, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameplayAbility:ApplyGameplayEffectContainer"))
        .Find("setnz *; add r*, r*")
        .Find("mov rsi, [rax+0x*]")
        .Get();

    TArray<FActiveGameplayEffectHandle> Handles;

    ApplyGameplayEffectContainer(this, &Handles, &TargetData, ApplicationTag, GameplayEffectLevel);

    return Handles;
}

float UFortGameplayAbility::GetCurrentAbilitySourceLevel()
{
    static auto GetCurrentAbilitySourceLevel = Anchor::Function<void (*)(UFortGameplayAbility*, float*)>(TEXT("UFortGameplayAbility::GetCurrentAbilitySourceLevel"))
        .String(TEXT("STAT_FortGA_Ranged_OnTargetDataReadyCB"))
        .Find("lea rdx, [rsp+0x*]; mov rcx, rsi; call 0x*; mov rax, [rsi]")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("STAT_FortGA_Ranged_OnTargetDataReadyCB"))
        .Find("lea rdx, [rsp+0x*]; mov rcx, rdi; call 0x*; mov rax, [rdi]")
        .Find("call 0x*")
        .Target()
        .Get();

    float Level = 0.0f;

    GetCurrentAbilitySourceLevel(this, &Level);

    return Level;
}

FGameplayTag UFortGameplayAbility::GetEffectContainerApplicationTag() const
{
    static auto EffectContainers = StaticClass()->FindPropertyByName(FName(TEXT("EffectContainers")));

    return EffectContainers ? *EffectContainers->ContainerPtrToValuePtr<FGameplayTag>(this) : FGameplayTag();
}

void UFortGameplayAbility::TagChangedActionCallback(const FGameplayTag Tag, bool bTagExists)
{
    static auto TagChangedActions = StaticClass()->FindPropertyByName(FName(TEXT("TagChangedActions")));
    static auto TagChangedActionStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameplayAbilityTagChangedAction"));

    if (!TagChangedActions || !TagChangedActionStruct)
        return;

    static auto Trigger = TagChangedActionStruct->FindPropertyByName(FName(TEXT("Trigger")));
    static auto Action = TagChangedActionStruct->FindPropertyByName(FName(TEXT("Action")));
    static auto ActionTag = TagChangedActionStruct->FindPropertyByName(FName(TEXT("Tag")));

    auto& Actions = *TagChangedActions->ContainerPtrToValuePtr<TArray<uint8>>(this);

    for (int32 Index = 0; Index < Actions.Num(); Index++)
    {
        auto ActionData = Actions.GetData() + Index * TagChangedActionStruct->GetPropertiesSize();

        if (!(ActionTag->GetPropertyValue_InContainer<FGameplayTag>(ActionData) == Tag))
            continue;

        const auto TriggerValue = *Trigger->ContainerPtrToValuePtr<EFortGameplayAbilityTagChangedEvent>(ActionData);
        const bool bTriggered = bTagExists ? (TriggerValue == EFortGameplayAbilityTagChangedEvent::NewlyAdded || TriggerValue == EFortGameplayAbilityTagChangedEvent::Increased) : (TriggerValue == EFortGameplayAbilityTagChangedEvent::FullyRemoved || TriggerValue == EFortGameplayAbilityTagChangedEvent::Decreased);

        if (bTriggered && *Action->ContainerPtrToValuePtr<EFortGameplayAbilityTagChangedActionType>(ActionData) == EFortGameplayAbilityTagChangedActionType::Cancel)
        {
            K2_CancelAbility();
            return;
        }
    }
}
