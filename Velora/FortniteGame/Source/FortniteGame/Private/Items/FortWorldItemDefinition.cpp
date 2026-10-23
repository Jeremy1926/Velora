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

bool UFortWorldItemDefinition::CanBeDropped() const
{
    static auto bCanBeDropped = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bCanBeDropped")));

    return bCanBeDropped->GetPropertyValue_InContainer(this);
}

bool UFortWorldItemDefinition::ShouldDropOnDeath() const
{
    static auto bDropOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDropOnDeath")));

    return bDropOnDeath->GetPropertyValue_InContainer(this);
}

float UFortWorldItemDefinition::GetMaxDurability(int32 ItemLevel) const
{
    static auto GetMaxDurability = Anchor::VirtualFunction<float (*)(const UFortWorldItemDefinition*, int32)>(TEXT("UFortWorldItemDefinition::GetMaxDurability"))
        .Exec(TEXT("/Script/FortniteGame.FortWorldItemDefinition:GetMaxDurability"))
        .Find("call [*+0x*]")
        .Get();

    return GetMaxDurability(this, ItemLevel);
}

const FDataTableCategoryHandle& UFortWorldItemDefinition::GetLootLevelData() const
{
    static auto LootLevelData = GetClass()->FindPropertyByName(FName(TEXT("LootLevelData")));

    return *LootLevelData->ContainerPtrToValuePtr<FDataTableCategoryHandle>(this);
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
