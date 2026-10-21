#pragma once

#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "Templates/SharedPointer.h"

struct FEnvQueryResult;

class AFortAthenaLivingWorldVolume : public AActor
{
public:
    static UClass* StaticClass();

    void OnEnvQueryFinished(const TSharedPtr<FEnvQueryResult>& Result);

    TArray<FVector>& GetPoints();
};
