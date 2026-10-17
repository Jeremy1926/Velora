#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "Math/Vector.h"

class UNetConnection;

struct FNetViewer
{
    UNetConnection* Connection;
    AActor* InViewer;
    AActor* ViewTarget;
    FVector ViewLocation;
    FVector ViewDir;

    FNetViewer(UNetConnection* InConnection, float DeltaSeconds);
};

class AWorldSettings : public AActor
{
public:
    TArray<FNetViewer>& GetReplicationViewers();
};
