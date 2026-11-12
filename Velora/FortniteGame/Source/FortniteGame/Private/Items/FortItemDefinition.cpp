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

UClass* UFortItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition"));

    return Class;
}

FString UFortItemDefinition::GetPersistentName() const
{
    static auto GetPersistentName = Anchor::VirtualFunction<FString* (*)(const UFortItemDefinition*, FString*)>(TEXT("UFortItemDefinition::GetPersistentName"))
        .Exec(TEXT("/Script/FortniteGame.FortItemDefinition:GetPersistentName"))
        .Find("call [rax+0x*]")
        .Get();

    FString PersistentName;
    GetPersistentName(this, &PersistentName);

    return PersistentName;
}

const uint8* UFortItemDefinition::GetItemComponentData(const UScriptStruct* ScriptStruct) const
{
    static auto DataList = StaticClass()->FindPropertyByName(FName(TEXT("DataList")));

    if (!DataList)
        return nullptr;

    for (const FConstStructView& Data : *DataList->ContainerPtrToValuePtr<TArray<FConstStructView>>(this))
    {
        if (Data.ScriptStruct == ScriptStruct && Data.StructMemory)
            return Data.StructMemory;
    }

    static auto GetDefaultInstance = Anchor::Optional<FConstStructView* (*)(FConstStructView*, const UScriptStruct*)>(TEXT("FItemComponentData::GetDefaultInstance"))
        .Native(TEXT("/Script/FortniteGame.FortWeaponItemDefinition:GetWeaponModSlots"))
        .Find("lea rcx, [rsp+0x20]; call 0x*; mov *, [rax+0x8]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortWeaponItemDefinition:GetWeaponModSlots"))
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rsp+0x20]; call 0x*; mov *, [rax+0x8]")
        .Find("call 0x*")
        .Target()
        .Get();

    FConstStructView DefaultInstance;

    return GetDefaultInstance ? GetDefaultInstance(&DefaultInstance, ScriptStruct)->StructMemory : nullptr;
}

int32 UFortItemDefinition::GetMinLevel() const
{
    static auto MinLevel = GetClass()->FindPropertyByName(FName(TEXT("MinLevel")));

    if (MinLevel)
        return *MinLevel->ContainerPtrToValuePtr<int32>(this);

    static auto LevelLimits = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_LevelLimits"));
    static auto LevelLimitsMinLevel = LevelLimits->FindPropertyByName(FName(TEXT("MinLevel")));

    return *LevelLimitsMinLevel->ContainerPtrToValuePtr<int32>(GetItemComponentData(LevelLimits));
}

const FText& UFortItemDefinition::GetDisplayName() const
{
    static auto DisplayName = GetClass()->FindPropertyByName(FName(TEXT("DisplayName"))) ? GetClass()->FindPropertyByName(FName(TEXT("DisplayName"))) : GetClass()->FindPropertyByName(FName(TEXT("ItemName")));

    return *DisplayName->ContainerPtrToValuePtr<FText>(this);
}

EFortItemType UFortItemDefinition::GetItemType() const
{
    static auto ItemType = GetClass()->FindPropertyByName(FName(TEXT("ItemType")));

    return *ItemType->ContainerPtrToValuePtr<EFortItemType>(this);
}

EFortRarity UFortItemDefinition::GetRarity() const
{
    static auto RarityInterfaceCall = Anchor::Address()
        .Exec(TEXT("/Script/FortniteGame.FortItemDefinition:GetRarity"))
        .Find("add rcx, 0x*; add [*+0x20], *")
        .Optional()
        .Get();

    if (RarityInterfaceCall)
    {
        static auto RarityInterfaceOffset = ::Anchor::At(RarityInterfaceCall).Imm();
        static auto RaritySlot = ::Anchor::At(RarityInterfaceCall).Find("call [*+0x*]").Disp();

        auto RarityInterface = (const uint8*)this + RarityInterfaceOffset;

        return (*(EFortRarity (**)(const void*))(*(const uint8* const*)RarityInterface + RaritySlot))(RarityInterface);
    }

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
    static auto NonVirtualClampItemLevel = Anchor::Optional<int32 (*)(const UFortItemDefinition*, int32)>(TEXT("UFortItemDefinition::ClampItemLevel"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("mov edx, [rbx+0x*]; mov rcx, [rbx+0x*]; mov [rbx+0x*], ebp; mov [rbx+0x*], ebp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("mov edx, [rbx+0x*]; mov rcx, [rbx+0x*]; call 0x*; mov rbp, [rsp+0x*]; mov rsi, [rsp+0x*]; mov [rbx+0x*], eax")
        .Find("call 0x*")
        .Target()
        .Get();

    if (NonVirtualClampItemLevel)
        return NonVirtualClampItemLevel(this, ItemLevel);

    static auto ClampItemLevel = Anchor::VirtualFunction<int32 (*)(const UFortItemDefinition*, int32)>(TEXT("UFortItemDefinition::ClampItemLevel"))
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("call [*]")
        .Or()
        .String(TEXT("%s HostUpgradeLevel: %i"))
        .Start()
        .Find("mov rdx, rax; movups [*], xmm0; call 0x*")
        .Find("call 0x*")
        .Call()
        .Find("call [*]")
        .Get();

    return ClampItemLevel(this, ItemLevel);
}

bool UFortItemDefinition::DoesAllowMultipleStacks() const
{
    static auto bAllowMultipleStacks = StaticClass()->FindPropertyByName(FName(TEXT("bAllowMultipleStacks")));
    static auto TraitsData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ItemizationCoreRuntime.ItemComponentData_Traits"));
    static auto Traits = TraitsData ? TraitsData->FindPropertyByName(FName(TEXT("Traits"))) : nullptr;

    if (!bAllowMultipleStacks && Traits)
    {
        static auto SingleStack = FGameplayTag::RequestGameplayTag(FName(TEXT("Item.Trait.SingleStack")), false);

        auto Data = GetItemComponentData(TraitsData);

        return !Data || !Traits->ContainerPtrToValuePtr<FGameplayTagContainer>(Data)->HasTag(SingleStack);
    }

    static auto bVirtualGetter = Anchor::Exists()
        .BoolGetter(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), (UBoolProperty*)bAllowMultipleStacks)
        .Get();

    if (bAllowMultipleStacks && !bVirtualGetter)
        return ((UBoolProperty*)bAllowMultipleStacks)->GetPropertyValue_InContainer(this);

    static auto DoesAllowMultipleStacks = Anchor::VirtualFunction<bool (*)(const UFortItemDefinition*)>(TEXT("UFortItemDefinition::DoesAllowMultipleStacks"))
        .BoolGetter(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), (UBoolProperty*)((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("bAllowMultipleStacks"))))
        .Get();

    return DoesAllowMultipleStacks(this);
}

int32 UFortItemDefinition::GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent) const
{
    static auto MaxStackSize = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemDefinition")))->FindPropertyByName(FName(TEXT("MaxStackSize")));
    static auto GetMaxStackSize = Anchor::VirtualFunction<int32 (*)(const UFortItemDefinition*, const UAbilitySystemComponent*)>(TEXT("UFortItemDefinition::GetMaxStackSize"))
        .If(MaxStackSize && MaxStackSize->GetSize() != sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "add rcx, %x", MaxStackSize, "cmp eax, 0x7ffffffe")
        .Or()
        .If(MaxStackSize && MaxStackSize->GetSize() != sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "add rcx, %x", MaxStackSize, "cvttss2si *, xmm0; cmp *, 0x7ffffffe")
        .Or()
        .If(MaxStackSize && MaxStackSize->GetSize() == sizeof(int32))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortItemDefinition"), "cmp dword ptr [*+%x], 0x7ffffffe", MaxStackSize)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortItemDefinition:GetMaxStackSize"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Get();

    return GetMaxStackSize(this, AbilitySystemComponent);
}

void UFortItemDefinition::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    static auto GameplayTags = GetClass()->FindPropertyByName(FName(TEXT("GameplayTags")));

    if (GameplayTags)
    {
        TagContainer.AppendTags(*GameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this));
        return;
    }

    static auto OwnedGameplayTags = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ItemizationCoreRuntime.ItemComponentData_OwnedGameplayTags"));
    static auto Tags = OwnedGameplayTags->FindPropertyByName(FName(TEXT("Tags")));

    TagContainer.AppendTags(*Tags->ContainerPtrToValuePtr<FGameplayTagContainer>(GetItemComponentData(OwnedGameplayTags)));
}
