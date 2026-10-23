#pragma once

#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "Math/Vector.h"

class UNetConnection;
class UScriptStruct;

struct FNetViewer
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FNetViewer(UNetConnection* InConnection, float DeltaSeconds);

    FVector GetViewLocation() const;

    FVector GetViewDir() const;

    UNetConnection* Connection;
    AActor* InViewer;
    AActor* ViewTarget;
};

template<>
class TArray<FNetViewer, FDefaultAllocator> : public TRuntimeSizedArray<FNetViewer>
{
public:
    void Reset()
    {
        ArrayNum = 0;
    }
};

class AWorldSettings : public AActor
{
public:
    TArray<FNetViewer>& GetReplicationViewers();
};
