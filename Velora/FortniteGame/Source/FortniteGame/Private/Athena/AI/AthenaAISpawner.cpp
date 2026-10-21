#include "pch.h"
#include "Athena/AI/AthenaAISpawner.h"
#include "Scanner.h"

int32 UAthenaAISpawner::RequestSpawn(UFortAthenaAISpawnerDataComponentList* AISpawnerComponentList, const FTransform& SpawnTransform)
{
    static auto RequestSpawn = Anchor::Function<int32 (*)(UAthenaAISpawner*, UFortAthenaAISpawnerDataComponentList*, const FTransform&)>(TEXT("UAthenaAISpawner::RequestSpawn"))
        .Native(TEXT("/Script/FortniteGame.AthenaAISpawner:RequestSpawn"))
        .Get();

    return RequestSpawn(this, AISpawnerComponentList, SpawnTransform);
}
