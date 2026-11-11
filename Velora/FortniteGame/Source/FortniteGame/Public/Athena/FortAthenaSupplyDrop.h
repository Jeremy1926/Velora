#pragma once

#include "Building/BuildingActor.h"
#include "Math/Vector.h"

class AFortPawn;
struct FFrame;
class AFortPickup;
class UFortWorldItemDefinition;

class AFortAthenaSupplyDrop : public ABuildingActor
{
public:
    static UClass* StaticClass();

    void RegisterSpecialActor(const uint8* InSpecialActorTag, const uint8* MinimapIconBrush, const uint8* MinimapIconScale, const uint8* CompassIconBrush, const uint8* CompassIconScale, bool bShouldDrawCompassIcon);

    UObject* GetWaveSpawnSoundCue() const;

    AFortPickup* SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const TGameValue<FVector>& GamePosition, const TGameValue<FVector>& GameDirection);

    void execSpawnPickup(FFrame& Stack, void* const Z_Param__Result);

    void execGetSupplyDropSpawnData(FFrame& Stack, void* const Z_Param__Result);

    void execGetSupplyDropSpeedData(FFrame& Stack, void* const Z_Param__Result);
};
