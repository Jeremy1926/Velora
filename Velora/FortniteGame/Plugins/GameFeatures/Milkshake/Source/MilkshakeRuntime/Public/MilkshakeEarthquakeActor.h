#pragma once

#include "Containers/Array.h"
#include "GameFramework/Actor.h"

struct FFrame;

class AMilkshakeEarthquakeActor : public AActor
{
public:
    static UClass* StaticClass();

    void RemoveNullsFromAsyncOverlappedActors();

    void execRemoveNullsFromAsyncOverlappedActors(FFrame& Stack, void* const Z_Param__Result);

    TArray<AActor*>& GetAsyncOverlappedActors();
};
