#pragma once

#include "Engine/DataAsset.h"
#include "Containers/Map.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "UObject/SoftObjectPtr.h"

class ABuildingProp;
class UFortVehicleItemDefinition;
enum class ESpawnActorCollisionHandlingMethod : uint8;

template<typename VectorType, typename RotatorType>
struct TVehiclePropReplacementData
{
    TSoftObjectPtr<UFortVehicleItemDefinition> FortVehicleItemDefinition;
    VectorType TransOffset;
    RotatorType RotOffset;
};

using FVehiclePropReplacementData = TVehiclePropReplacementData<FVector, FRotator>;

class UFortValetGameFeatureData : public UPrimaryDataAsset
{
public:
    static UClass* StaticClass();

    const TMap<TSoftClassPtr<ABuildingProp>, FVehiclePropReplacementData>& GetPropReplacements() const;

    const TSoftClassPtr<UClass>& GetVehicleSpawnerClass() const;

    ESpawnActorCollisionHandlingMethod GetSpawnActorCollisionHandlingMethod() const;
};
