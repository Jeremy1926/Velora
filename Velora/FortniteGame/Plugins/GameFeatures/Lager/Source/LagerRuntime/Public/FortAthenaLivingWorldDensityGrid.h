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
    bool IsInitialized() const;

    FIntVector GetCellCoordinates(const FVector& Location) const;

    bool AddActor(AActor* InActor);

    void UpdateActors(int32 MaxActorUpdates);

    TMap<FIntVector, TArray<int32>>& GetCellToActorDataIndices();

    TMap<FObjectKey, int32>& GetActorToDataIndexMap();

    TSparseArray<FFortAthenaLivingWorldDensityData>& GetActorDataArray();

private:
    void SetActorDataCell(int32 ActorDataIndex, const FIntVector& Cell);

    static int32 NextActorUpdateIndex;
};
