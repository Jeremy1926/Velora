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

EFortItemType UFortItemDefinition::GetItemType() const
{
    static auto ItemType = GetClass()->FindPropertyByName(FName(TEXT("ItemType")));

    return *ItemType->ContainerPtrToValuePtr<EFortItemType>(this);
}

EFortRarity UFortItemDefinition::GetRarity() const
{
    static auto GetRarity = Anchor::Slot(TEXT("UFortItemDefinition::GetRarity"))
        .Native(TEXT("/Script/FortniteGame.FortItem:GetRarity"))
        .Find("jmp [*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortItemDefinition:GetRarity"))
        .Find("call [*+0x*]")
        .Get();

    return Scanner::Virtual<EFortRarity (*)(const UFortItemDefinition*)>(this, GetRarity)(this);
}

int32 UFortItemDefinition::ClampItemLevel(int32 ItemLevel) const
{
    static auto ClampItemLevel = Anchor::Slot(TEXT("UFortItemDefinition::ClampItemLevel"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("call [*]")
        .Get();

    return Scanner::Virtual<int32 (*)(const UFortItemDefinition*, int32)>(this, ClampItemLevel)(this, ItemLevel);
}

bool UFortItemDefinition::DoesAllowMultipleStacks() const
{
    static auto DoesAllowMultipleStacks = []
    {
        auto bAllowMultipleStacks = (UBoolProperty*)((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("bAllowMultipleStacks")));
        auto Offset = bAllowMultipleStacks->GetOffset_ForInternal() + bAllowMultipleStacks->GetByteOffset();
        auto Bit = std::countr_zero(bAllowMultipleStacks->GetFieldMask());
        auto Getter = Bit ? std::format("mov* *, *[rcx+{:#x}]; shr al, {:#x}; and *", Offset, Bit) : std::format("mov* *, *[rcx+{:#x}]; and *", Offset);

        return Anchor::Slot(TEXT("UFortItemDefinition::DoesAllowMultipleStacks"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), [&](const Anchor::FCode& Entry) { return Entry.Is(Getter); })
            .Get();
    }();

    return Scanner::Virtual<bool (*)(const UFortItemDefinition*)>(this, DoesAllowMultipleStacks)(this);
}

int32 UFortItemDefinition::GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent) const
{
    static auto GetMaxStackSize = []
    {
        auto MaxStackSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("MaxStackSize")))->GetOffset_ForInternal();
        auto Compare = std::format("cmp dword ptr [*+{:#x}], 0x7ffffffe", MaxStackSize);

        return Anchor::Slot(TEXT("UFortItemDefinition::GetMaxStackSize"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), [&](const Anchor::FCode& Entry) { return Entry.Start().Has(Compare); })
            .Get();
    }();

    return Scanner::Virtual<int32 (*)(const UFortItemDefinition*, const UAbilitySystemComponent*)>(this, GetMaxStackSize)(this, AbilitySystemComponent);
}

void UFortItemDefinition::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GameplayTags = GetClass()->FindPropertyByName(FName(TEXT("GameplayTags")));

    TagContainer.AppendTags(*GameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this));
}
