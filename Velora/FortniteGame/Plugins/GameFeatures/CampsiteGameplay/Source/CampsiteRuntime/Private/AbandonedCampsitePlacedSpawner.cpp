#include "pch.h"
#include "AbandonedCampsitePlacedSpawner.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AAbandonedCampsitePlacedSpawner::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.AbandonedCampsitePlacedSpawner"));

    return Class;
}

void AAbandonedCampsitePlacedSpawner::SpawnCampsite()
{
    static auto SpawnCampsite = Anchor::VirtualFunction<void (*)(AAbandonedCampsitePlacedSpawner*)>(TEXT("AAbandonedCampsitePlacedSpawner::SpawnCampsite_Implementation"))
        .Exec(TEXT("/Script/CampsiteRuntime.AbandonedCampsitePlacedSpawner:SpawnCampsite"))
        .Find("jmp [rax+0x*]")
        .Get();

    SpawnCampsite(this);
}
