#pragma once

#include "Containers/Array.h"
#include "Containers/ArrayView.h"
#include "Containers/Set.h"
#include "Math/Box.h"
#include "Math/Vector.h"

class AController;
class AFortAthenaAIBotController;
class AFortGameStateAthena;
class AFortPlayerPawn;
class UScriptStruct;
class UWorld;

struct FSimpleBVTreeElement
{
    int32 Index;
    FVector Location;
};

template<typename ElementType>
class TBVTree
{
public:
    void RecreateTree(const TArray<ElementType>& InElements);

private:
    TArray<int16> Nodes;
    TArray<FBox> NodeBoundingBoxes;
    TArray<ElementType> Elements;
};

struct FFortAthenaLivingWorldPlayerTracker
{
    static UScriptStruct* StaticStruct();

    TSet<const AFortAthenaAIBotController*>& GetTrackedBotControllers();

    TArray<const AFortPlayerPawn*>& GetTrackedPlayerPawns();

    TArray<const AController*>& GetTrackedPlayerControllers();

    TBVTree<FSimpleBVTreeElement>& GetTrackedPlayerBVTree();

    int32& GetMinNumberOfPlayers();

    float& GetTimeSinceLastRefresh();

    bool& GetbHasSelectedPlayerBots();

    void UpdateTimeSinceLastRefresh(float DeltaTime);

    void RefreshTree(UWorld* World, const AFortGameStateAthena* GameState);

    void GetNumberOfPlayerPawnInRange(const FVector& Origin, float MaxRadius, const TArrayView<const float>& SquaredRadii, const TArrayView<int32>& OutNumberOfPlayerInRadius, TArray<FSimpleBVTreeElement>& ScratchPad);
};
