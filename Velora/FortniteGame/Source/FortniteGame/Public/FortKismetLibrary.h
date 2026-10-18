#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Vector.h"

class AFortPlayerController;
class UFortWorldItemDefinition;
class USoundBase;

class UFortKismetLibrary : public UBlueprintFunctionLibrary
{
public:
    static void K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive);

    static void BroadcastSoundAtLocation(UObject* WorldContextObject, USoundBase* InSound, const FVector& Location, float VolumeMultiplier, float PitchMultiplier);
};
