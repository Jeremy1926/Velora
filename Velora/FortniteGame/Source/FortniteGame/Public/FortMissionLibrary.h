#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

class AFortPlayerPawn;
struct FFrame;

class UFortMissionLibrary : public UBlueprintFunctionLibrary
{
public:
    static bool TeleportPlayerPawn(const UObject* WorldContextObject, AFortPlayerPawn* PlayerPawn, const FVector& DestLocation, const FRotator& DestRotation, bool bIgnoreCollision, bool bIgnoreSupplementalKillVolumeSweep);

    void execTeleportPlayerPawn(FFrame& Stack, void* const Z_Param__Result);
};
