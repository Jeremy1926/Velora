#pragma once

#include "Components/ActorComponent.h"

class AFortPawn;

class UFortPawnComponent_Convert : public UActorComponent
{
public:
    static UClass* StaticClass();

    void ConvertPawn(AFortPawn* PawnToConvert);

    void InternalConvertPawn(AFortPawn* PawnToConvert, UFortPawnComponent_Convert* ConvertedPawnConvertComponent);
};
