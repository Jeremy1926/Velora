#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "UObject/ScriptDelegates.h"

class AFortAthenaMapInfo;
class UScriptStruct;
class UWorld;

enum class EAirCraftBehavior : uint8
{
    Default,
    OpposingAirCraftForEachTeam,
    FlyTowardFirstCircleCenter,
    NoAircraft,
};

struct FAircraftFlightInfo
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();
};

template<>
class TArray<FAircraftFlightInfo, FDefaultAllocator> : public TRuntimeSizedArray<FAircraftFlightInfo>
{
};

class AFortAthenaAircraft : public AActor
{
public:
    static UClass* StaticClass();

    static AFortAthenaAircraft* SpawnAircraft(UWorld* World, int32 AircraftIndex);

    void StartFlightPath(float StartTime);

    FMulticastScriptDelegate& GetOnDropZoneEntered();

    FMulticastScriptDelegate& GetOnDropZoneExited();

    FMulticastScriptDelegate& GetOnFlightEnded();

    const FAircraftFlightInfo& GetFlightInfo() const;

    bool IsInDropZone() const;

    int32& GetAircraftIndex();
};
