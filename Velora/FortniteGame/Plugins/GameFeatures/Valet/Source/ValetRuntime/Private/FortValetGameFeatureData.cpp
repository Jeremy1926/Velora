#include "pch.h"
#include "FortValetGameFeatureData.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortValetGameFeatureData::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ValetRuntime.FortValetGameFeatureData"));

    return Class;
}

TArray<TPair<const TSoftClassPtr<ABuildingProp>*, FVehiclePropReplacementData>> UFortValetGameFeatureData::GetPropReplacements() const
{
    static auto PropReplacements = StaticClass()->FindPropertyByName(FName(TEXT("PropReplacements")));
    static auto PropReplacementDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/ValetRuntime.VehiclePropReplacementData"));
    static auto FortVehicleItemDefinition = PropReplacementDataStruct->FindPropertyByName(FName(TEXT("FortVehicleItemDefinition")));
    static auto TransOffset = PropReplacementDataStruct->FindPropertyByName(FName(TEXT("TransOffset")));
    static auto RotOffset = PropReplacementDataStruct->FindPropertyByName(FName(TEXT("RotOffset")));
    static auto ValueAlignment = PropReplacementDataStruct->GetMinAlignment();
    static auto PairAlignment = FMath::Max(FMath::Max(ValueAlignment, (int32)alignof(FSoftObjectPtr)), (int32)sizeof(int32));
    static auto ValueOffset = (TSoftClassPtr<ABuildingProp>::GetTypeSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + PropReplacementDataStruct->GetPropertiesSize() + PairAlignment - 1) & ~(PairAlignment - 1);
    static auto ElementSize = (PairSize + 2 * (int32)sizeof(int32) + PairAlignment - 1) & ~(PairAlignment - 1);

    auto& Elements = *PropReplacements->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);
    auto ElementData = *PropReplacements->ContainerPtrToValuePtr<const uint8*>(this);
    TArray<TPair<const TSoftClassPtr<ABuildingProp>*, FVehiclePropReplacementData>> Pairs;

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        if (!Elements.IsAllocated(Index))
            continue;

        auto Element = ElementData + Index * ElementSize;
        auto Value = Element + ValueOffset;

        Pairs.Add({ (const TSoftClassPtr<ABuildingProp>*)Element, FVehiclePropReplacementData{ *FortVehicleItemDefinition->ContainerPtrToValuePtr<TSoftObjectPtr<UFortVehicleItemDefinition>>(Value), TransOffset->GetPropertyValue_InContainer<FVector>(Value), RotOffset->GetPropertyValue_InContainer<FRotator>(Value) } });
    }

    return Pairs;
}

const TSoftClassPtr<UClass>& UFortValetGameFeatureData::GetVehicleSpawnerClass() const
{
    static auto VehicleSpawnerClass = StaticClass()->FindPropertyByName(FName(TEXT("VehicleSpawnerClass")));

    return *VehicleSpawnerClass->ContainerPtrToValuePtr<TSoftClassPtr<UClass>>(this);
}

ESpawnActorCollisionHandlingMethod UFortValetGameFeatureData::GetSpawnActorCollisionHandlingMethod() const
{
    static auto SpawnActorCollisionHandlingMethod = StaticClass()->FindPropertyByName(FName(TEXT("SpawnActorCollisionHandlingMethod")));

    return *SpawnActorCollisionHandlingMethod->ContainerPtrToValuePtr<ESpawnActorCollisionHandlingMethod>(this);
}
