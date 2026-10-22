#include "pch.h"
#include "Items/FortItemDefinition.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

template<>
UEnum* StaticEnum<EFortItemType>()
{
    static auto Enum = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EFortItemType"));

    return Enum;
}

int32 UFortItemDefinition::GetMinLevel() const
{
    static auto MinLevel = GetClass()->FindPropertyByName(FName(TEXT("MinLevel")));

    return *MinLevel->ContainerPtrToValuePtr<int32>(this);
}

const FText& UFortItemDefinition::GetDisplayName() const
{
    static auto DisplayName = GetClass()->FindPropertyByName(FName(TEXT("DisplayName")));

    return *DisplayName->ContainerPtrToValuePtr<FText>(this);
}

EFortItemType UFortItemDefinition::GetItemType() const
{
    static auto ItemType = GetClass()->FindPropertyByName(FName(TEXT("ItemType")));

    return *ItemType->ContainerPtrToValuePtr<EFortItemType>(this);
}

EFortRarity UFortItemDefinition::GetRarity() const
{
    static auto GetRarity = Anchor::VirtualFunction<EFortRarity (*)(const UFortItemDefinition*)>(TEXT("UFortItemDefinition::GetRarity"))
        .Native(TEXT("/Script/FortniteGame.FortItem:GetRarity"))
        .Find("jmp [*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortItemDefinition:GetRarity"))
        .Find("call [*+0x*]")
        .Get();

    return GetRarity(this);
}

int32 UFortItemDefinition::ClampItemLevel(int32 ItemLevel) const
{
    static auto ClampItemLevel = Anchor::VirtualFunction<int32 (*)(const UFortItemDefinition*, int32)>(TEXT("UFortItemDefinition::ClampItemLevel"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("call [*]")
        .Get();

    return ClampItemLevel(this, ItemLevel);
}

bool UFortItemDefinition::DoesAllowMultipleStacks() const
{
    static auto DoesAllowMultipleStacks = Anchor::VirtualFunction<bool (*)(const UFortItemDefinition*)>(TEXT("UFortItemDefinition::DoesAllowMultipleStacks"))
        .BoolGetter(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), (UBoolProperty*)((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("bAllowMultipleStacks"))))
        .Get();

    return DoesAllowMultipleStacks(this);
}

int32 UFortItemDefinition::GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent) const
{
    static auto MaxStackSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("MaxStackSize")));
    static auto GetMaxStackSize = Anchor::VirtualFunction<int32 (*)(const UFortItemDefinition*, const UAbilitySystemComponent*)>(TEXT("UFortItemDefinition::GetMaxStackSize"))
        .If(MaxStackSize->GetSize() != sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "add rcx, %x", MaxStackSize, "cmp eax, 0x7ffffffe")
        .Or()
        .If(MaxStackSize->GetSize() != sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "add rcx, %x", MaxStackSize, "cvttss2si *, xmm0; cmp *, 0x7ffffffe")
        .Or()
        .If(MaxStackSize->GetSize() == sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "cmp dword ptr [*+%x], 0x7ffffffe", MaxStackSize)
        .Get();

    return GetMaxStackSize(this, AbilitySystemComponent);
}

void UFortItemDefinition::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GameplayTags = GetClass()->FindPropertyByName(FName(TEXT("GameplayTags")));

    TagContainer.AppendTags(*GameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this));
}
