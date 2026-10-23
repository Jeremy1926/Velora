#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/SparseArray.h"
#include "Math/IntVector.h"
#include "Math/Vector.h"
#include "UObject/ObjectKey.h"
#include "UObject/WeakObjectPtr.h"

class AActor;

struct FFortAthenaLivingWorldDensityData
{
    FWeakObjectPtr Actor;
    TArray<FIntVector> OccupiedCells;
};

struct FFortAthenaLivingWorldDensityGrid
{
    struct FActorDensityRange
    {
        FWeakObjectPtr Actor;
        float DensityRange;
    };

    static bool HasActorDensityValues();

    bool IsInitialized() const;

    FIntVector GetCellCoordinates(const FVector& Location) const;

    bool AddActor(AActor* InActor, float DensityValue, float DensityRange);

    void RemoveActor(AActor* InActor);

    float GetActorDensityValue(AActor* InActor);

    void UpdateActorDensityValue(AActor* InActor, float DensityValue);

    void UpdateActors(int32 MaxActorUpdates);

    TMap<FIntVector, TArray<int32>>& GetCellToActorDataIndices();

    TMap<FObjectKey, int32>& GetActorToDataIndexMap();

    TSparseArray<FFortAthenaLivingWorldDensityData>& GetActorDataArray();

private:
    void SetActorDataCell(int32 ActorDataIndex, const FIntVector& Cell);

    static int32 NextActorUpdateIndex;

    static TArray<FActorDensityRange> ActorDensityRanges;
};
