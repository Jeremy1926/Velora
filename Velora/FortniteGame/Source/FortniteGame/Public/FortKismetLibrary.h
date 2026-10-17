#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

class AFortPlayerController;
class UFortWorldItemDefinition;

class UFortKismetLibrary : public UBlueprintFunctionLibrary
{
public:
    static void K2_GiveItemToPlayer(AFortPlayerController* PlayerController, UFortWorldItemDefinition* ItemDefinition, int32 NumberToGive);
};
