#include "pch.h"
#include "Items/FortMontageItemDefinitionBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

const TAssetPtr<UObject>& UFortMontageItemDefinitionBase::GetAnimation() const
{
    static auto Animation = GetClass()->FindPropertyByName(FName(TEXT("Animation")));

    return *Animation->ContainerPtrToValuePtr<TAssetPtr<UObject>>(this);
}
