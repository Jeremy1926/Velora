#include "pch.h"
#include "Items/FortItemDefinition.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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
    static auto GetRarity = Scanner::Native(TEXT("/Script/FortniteGame.FortItem:GetRarity")).Find("jmp [*]").Slot(TEXT("UFortItemDefinition::GetRarity"));

    return (*(EFortRarity (**)(const UFortItemDefinition*))(*(const uint8_t**)this + GetRarity))(this);
}

int32 UFortItemDefinition::ClampItemLevel(int32 ItemLevel) const
{
    static auto ClampItemLevel = Scanner::String(TEXT("%s HostUpgradeLevel: %i")).Start().Find("mov rdx, rax; call 0x*").Find("call 0x*").Call().Find("call [*]").Slot(TEXT("UFortItemDefinition::ClampItemLevel"));

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const UFortItemDefinition*, int32))(VTable + ClampItemLevel))(this, ItemLevel);
}

bool UFortItemDefinition::DoesAllowMultipleStacks() const
{
    static auto DoesAllowMultipleStacks = []
    {
        auto bAllowMultipleStacks = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("bAllowMultipleStacks")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortItemDefinition"));
        auto Getter = std::format("mov *, [rcx+{:#x}]; and *", bAllowMultipleStacks);
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Is(Getter))
            Index++;

        return Scanner::Slot(TEXT("UFortItemDefinition::DoesAllowMultipleStacks"), Index * (int32)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const UFortItemDefinition*))(VTable + DoesAllowMultipleStacks))(this);
}

int32 UFortItemDefinition::GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent) const
{
    static auto GetMaxStackSize = []
    {
        auto MaxStackSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("MaxStackSize")))->GetOffset_ForInternal();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortItemDefinition"));
        auto Compare = std::format("cmp dword ptr [*+{:#x}], 0x7ffffffe", MaxStackSize);
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Start().Has(Compare))
            Index++;

        return Scanner::Slot(TEXT("UFortItemDefinition::GetMaxStackSize"), Index * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const UFortItemDefinition*, const UAbilitySystemComponent*))(VTable + GetMaxStackSize))(this, AbilitySystemComponent);
}

void UFortItemDefinition::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GameplayTags = GetClass()->FindPropertyByName(FName(TEXT("GameplayTags")));

    TagContainer.AppendTags(*GameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this));
}
