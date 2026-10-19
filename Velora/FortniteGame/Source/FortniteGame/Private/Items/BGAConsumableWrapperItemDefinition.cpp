#include "pch.h"
#include "Items/BGAConsumableWrapperItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UBGAConsumableWrapperItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BGAConsumableWrapperItemDefinition"));

    return Class;
}

const TAssetPtr<UClass>& UBGAConsumableWrapperItemDefinition::GetConsumableClass() const
{
    static auto ConsumableClass = GetClass()->FindPropertyByName(FName(TEXT("ConsumableClass")));

    return *ConsumableClass->ContainerPtrToValuePtr<TAssetPtr<UClass>>(this);
}
