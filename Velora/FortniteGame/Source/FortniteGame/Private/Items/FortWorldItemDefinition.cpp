#include "pch.h"
#include "Items/FortWorldItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"
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

    if (bSupportsQuickbarFocus)
        return bSupportsQuickbarFocus->GetPropertyValue_InContainer(this);

    static auto TraitsData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ItemizationCoreRuntime.ItemComponentData_Traits"));
    static auto Traits = TraitsData ? TraitsData->FindPropertyByName(FName(TEXT("Traits"))) : nullptr;
    static auto DisallowQuickbarFocus = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.Trait.DisallowQuickbarFocus")), false);

    auto Data = Traits ? GetItemComponentData(TraitsData) : nullptr;

    return !Data || !Traits->ContainerPtrToValuePtr<FGameplayTagContainer>(Data)->HasTag(DisallowQuickbarFocus);
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

    if (PickupComponent)
        return bCanBeDroppedFromInventory->GetPropertyValue_InContainer(PickupComponent);

    static auto PickupData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_Pickup"));
    static auto bDataCanBeDroppedFromInventory = PickupData ? (UBoolProperty*)PickupData->FindPropertyByName(FName(TEXT("bCanBeDroppedFromInventory"))) : nullptr;

    auto Data = bDataCanBeDroppedFromInventory ? GetItemComponentData(PickupData) : nullptr;

    return Data ? bDataCanBeDroppedFromInventory->GetPropertyValue_InContainer(Data) : true;
}

bool UFortWorldItemDefinition::ShouldDropOnDeath() const
{
    static auto bDropOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDropOnDeath")));
    static auto PickupComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponent_Pickup"));
    static auto bComponentDropOnDeath = PickupComponentClass ? (UBoolProperty*)PickupComponentClass->FindPropertyByName(FName(TEXT("bDropOnDeath"))) : nullptr;

    if (bDropOnDeath)
        return bDropOnDeath->GetPropertyValue_InContainer(this);

    auto PickupComponent = bComponentDropOnDeath ? GetPickupItemComponent() : nullptr;

    if (PickupComponent)
        return bComponentDropOnDeath->GetPropertyValue_InContainer(PickupComponent);

    static auto PickupData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_Pickup"));
    static auto bDataDropOnDeath = PickupData ? (UBoolProperty*)PickupData->FindPropertyByName(FName(TEXT("bDropOnDeath"))) : nullptr;

    auto Data = bDataDropOnDeath ? GetItemComponentData(PickupData) : nullptr;

    return Data ? bDataDropOnDeath->GetPropertyValue_InContainer(Data) : true;
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

    static auto LootLevelMapping = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_LootLevelMapping"));
    static auto LootLevelCategoryHandle = LootLevelMapping->FindPropertyByName(FName(TEXT("LootLevelCategoryHandle")));

    return *LootLevelCategoryHandle->ContainerPtrToValuePtr<FDataTableCategoryHandle>(GetItemComponentData(LootLevelMapping));
}

template<>
UEnum* StaticEnum<EWorldItemDropBehavior>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EWorldItemDropBehavior"));

    return Enum;
}

EWorldItemDropBehavior UFortWorldItemDefinition::GetDropBehavior() const
{
    static auto DropBehavior = StaticClass()->FindPropertyByName(FName(TEXT("DropBehavior")));
    static auto PickupComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponent_Pickup"));
    static auto ComponentDropBehavior = PickupComponentClass ? PickupComponentClass->FindPropertyByName(FName(TEXT("DropBehavior"))) : nullptr;
    static auto PickupData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_Pickup"));
    static auto DataDropBehavior = PickupData ? PickupData->FindPropertyByName(FName(TEXT("DropBehavior"))) : nullptr;
    static auto DropAsPickup = (EWorldItemDropBehavior)StaticEnum<EWorldItemDropBehavior>()->GetValueByName(FName(TEXT("EWorldItemDropBehavior::DropAsPickup")));

    if (DropBehavior)
        return *DropBehavior->ContainerPtrToValuePtr<EWorldItemDropBehavior>(this);

    if (ComponentDropBehavior)
    {
        auto PickupComponent = GetPickupItemComponent();

        return PickupComponent ? *ComponentDropBehavior->ContainerPtrToValuePtr<EWorldItemDropBehavior>(PickupComponent) : DropAsPickup;
    }

    if (DataDropBehavior)
    {
        auto Data = GetItemComponentData(PickupData);

        return Data ? *DataDropBehavior->ContainerPtrToValuePtr<EWorldItemDropBehavior>(Data) : DropAsPickup;
    }

    static auto GetDropBehavior = Anchor::VirtualFunction<EWorldItemDropBehavior (*)(const UFortWorldItemDefinition*)>(TEXT("UFortWorldItemDefinition::GetDropBehavior"))
        .At(DETOUR_ORIGINAL(AFortAthenaAIBotController, DropInventoryItems))
        .Find("mov rax, [rbx]; mov rcx, rbx; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    return GetDropBehavior(this);
}
