#include "pch.h"
#include "Items/FortWorldItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortWorldItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWorldItemDefinition"));

    return Class;
}

bool UFortWorldItemDefinition::ServerExecute(UFortWorldItem* Item, AFortPlayerController* PlayerController) const
{
    static auto ServerExecute = Anchor::VirtualFunction<bool (*)(const UFortWorldItemDefinition*, UFortWorldItem*, AFortPlayerController*)>(TEXT("UFortWorldItemDefinition::ServerExecute"))
        .String(TEXT("STAT_UFortWeaponItemDefinition_ServerExecute"))
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortWeaponItemDefinition"))
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortPawn:EquipWeaponDefinition"))
        .Log(TEXT("AFortPawn::EquipWeaponDefinition"))
        .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortWeaponItemDefinition"))
        .Get();

    return ServerExecute(this, Item, PlayerController);
}

bool UFortWorldItemDefinition::SupportsQuickbarFocus() const
{
    static auto bSupportsQuickbarFocus = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bSupportsQuickbarFocus")));

    return bSupportsQuickbarFocus->GetPropertyValue_InContainer(this);
}

UObject* UFortWorldItemDefinition::GetPickupItemComponent() const
{
    static auto PickupComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponent_Pickup"));
    static auto ComponentContainer = GetClass()->FindPropertyByName(FName(TEXT("ComponentContainer")));

    if (!PickupComponentClass || !ComponentContainer)
        return nullptr;

    for (auto Component : *ComponentContainer->ContainerPtrToValuePtr<TArray<UObject*>>(this))
    {
        if (Component && Component->IsA(PickupComponentClass))
            return Component;
    }

    return nullptr;
}

bool UFortWorldItemDefinition::CanBeDropped() const
{
    static auto bCanBeDropped = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCanBeDropped")));
    static auto PickupComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponent_Pickup"));
    static auto bCanBeDroppedFromInventory = PickupComponentClass ? (UBoolProperty*)PickupComponentClass->FindPropertyByName(FName(TEXT("bCanBeDroppedFromInventory"))) : nullptr;

    if (bCanBeDropped)
        return bCanBeDropped->GetPropertyValue_InContainer(this);

    auto PickupComponent = bCanBeDroppedFromInventory ? GetPickupItemComponent() : nullptr;

    return PickupComponent ? bCanBeDroppedFromInventory->GetPropertyValue_InContainer(PickupComponent) : true;
}

bool UFortWorldItemDefinition::ShouldDropOnDeath() const
{
    static auto bDropOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDropOnDeath")));
    static auto PickupComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponent_Pickup"));
    static auto bComponentDropOnDeath = PickupComponentClass ? (UBoolProperty*)PickupComponentClass->FindPropertyByName(FName(TEXT("bDropOnDeath"))) : nullptr;

    if (bDropOnDeath)
        return bDropOnDeath->GetPropertyValue_InContainer(this);

    auto PickupComponent = bComponentDropOnDeath ? GetPickupItemComponent() : nullptr;

    return PickupComponent ? bComponentDropOnDeath->GetPropertyValue_InContainer(PickupComponent) : true;
}

float UFortWorldItemDefinition::GetMaxDurability(int32 ItemLevel) const
{
    static auto GetMaxDurability = Anchor::VirtualFunction<float (*)(const UFortWorldItemDefinition*, int32)>(TEXT("UFortWorldItemDefinition::GetMaxDurability"))
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Find("call [*+0x*]")
        .Get();

    return GetMaxDurability(this, ItemLevel);
}

const FDataTableCategoryHandle& UFortWorldItemDefinition::GetLootLevelData() const
{
    static auto LootLevelData = GetClass()->FindPropertyByName(FName(TEXT("LootLevelData")));

    if (LootLevelData)
        return *LootLevelData->ContainerPtrToValuePtr<FDataTableCategoryHandle>(this);

    static auto GetLootLevelData = Anchor::Function<const FDataTableCategoryHandle* (*)(const UFortWorldItemDefinition*)>(TEXT("UFortWorldItemDefinition::GetLootLevelData"))
        .String(TEXT("UFortLootLevel::GetItemLevel"))
        .Start()
        .Caller()
        .Find("mov rcx, rsi; call 0x*; mov edx, ebx; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return *GetLootLevelData(this);
}

template<>
UEnum* StaticEnum<EWorldItemDropBehavior>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EWorldItemDropBehavior"));

    return Enum;
}

EWorldItemDropBehavior UFortWorldItemDefinition::GetDropBehavior() const
{
    static auto GetDropBehavior = Anchor::VirtualFunction<EWorldItemDropBehavior (*)(const UFortWorldItemDefinition*)>(TEXT("UFortWorldItemDefinition::GetDropBehavior"))
        .At(DETOUR_ORIGINAL(AFortAthenaAIBotController, DropInventoryItems))
        .Find("mov rax, [rbx]; mov rcx, rbx; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    return GetDropBehavior(this);
}
