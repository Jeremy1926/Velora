#include "pch.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortInventory.h"
#include "Weapons/FortWeaponModItemDefinition.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UScriptStruct* FFortBaseWeaponStats::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortBaseWeaponStats"));

    return Struct;
}

const int32& FFortBaseWeaponStats::GetBaseLevel() const
{
    static auto BaseLevel = StaticStruct()->FindPropertyByName(FName(TEXT("BaseLevel")));

    return *BaseLevel->ContainerPtrToValuePtr<int32>(this);
}

const int32& FFortBaseWeaponStats::GetClipSize() const
{
    static auto ClipSize = StaticStruct()->FindPropertyByName(FName(TEXT("ClipSize")));

    return *ClipSize->ContainerPtrToValuePtr<int32>(this);
}

const float& FFortBaseWeaponStats::GetClipScale() const
{
    static auto ClipScale = StaticStruct()->FindPropertyByName(FName(TEXT("ClipScale")));

    return *ClipScale->ContainerPtrToValuePtr<float>(this);
}

UClass* UFortWeaponItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeaponItemDefinition"));

    return Class;
}

int32 UFortWeaponItemDefinition::GetBaseClipSizeWithMods(const UObject* WorldContextObject, const FFortItemEntry* ItemEntry, const FFortBaseWeaponStats* WeaponStats, int32 Level) const
{
    static auto GetBaseClipSizeWithMods = Anchor::Function<int32 (*)(const UFortWeaponItemDefinition*, const UObject*, const FFortItemEntry*, const FFortBaseWeaponStats*, int32, int32)>(TEXT("UFortWeaponItemDefinition::GetBaseClipSizeWithMods"))
        .Exec(TEXT("/Script/FortniteGame.FortWeaponItemDefinition:TryGetBaseClipSizeWithMods"))
        .Find("mov [rsp+0x20], *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortWeaponItemDefinition:TryGetBaseClipSizeWithMods"))
        .Find("mov r8, *; mov [rsp+0x20], *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortWeaponItemDefinition:TryGetBaseClipSizeWithMods"))
        .Find("mov r8, *; mov [rsp+0x20], *; mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Optional()
        .Get();

    if (!WeaponStats)
        return 0;

    if (GetBaseClipSizeWithMods)
        return GetBaseClipSizeWithMods(this, WorldContextObject, ItemEntry, WeaponStats, Level, -1);

    return (int32)((FMath::Max((float)Level - (float)WeaponStats->GetBaseLevel(), 0.0f) * WeaponStats->GetClipScale() + 1.0f) * (float)WeaponStats->GetClipSize());
}

const FDataTableRowHandle& UFortWeaponItemDefinition::GetWeaponStatHandle() const
{
    static auto WeaponStatHandle = GetClass()->FindPropertyByName(FName(TEXT("WeaponStatHandle")));

    return *WeaponStatHandle->ContainerPtrToValuePtr<FDataTableRowHandle>(this);
}

const TArray<FFortWeaponModSlot>& UFortWeaponItemDefinition::GetWeaponModSlots() const
{
    static auto WeaponModSlots = StaticClass()->FindPropertyByName(FName(TEXT("WeaponModSlots")));

    if (WeaponModSlots)
        return *WeaponModSlots->ContainerPtrToValuePtr<TArray<FFortWeaponModSlot>>(this);

    static auto WeaponModSlotsData = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortItemComponentData_WeaponModSlots"));
    static auto DataWeaponModSlots = WeaponModSlotsData->FindPropertyByName(FName(TEXT("WeaponModSlots")));

    return *DataWeaponModSlots->ContainerPtrToValuePtr<TArray<FFortWeaponModSlot>>(GetItemComponentData(WeaponModSlotsData));
}

bool UFortWeaponItemDefinition::SetDefaultWeaponModSlotsIfNeeded(FFortItemEntry& ItemEntry) const
{
    if (GetWeaponModSlots().Num() > 0 && ItemEntry.GetWeaponModSlots().Num() == 0)
    {
        ItemEntry.SetWeaponModSlots(GetWeaponModSlots());
        return true;
    }

    return false;
}

FGameplayTag UFortWeaponItemDefinition::GetWeaponModBucketTag() const
{
    FGameplayTagContainer GameplayTags;
    GetOwnedGameplayTags(GameplayTags);

    for (const FGameplayTag& GameplayTag : GameplayTags.GetGameplayTagArray())
    {
        const FString TagString = GameplayTag.ToString();

        if (TagString == FString(TEXT("ModSetBucket")) || TagString.StartsWith(TEXT("ModSetBucket.")))
            return GameplayTag;
    }

    return FGameplayTag();
}
