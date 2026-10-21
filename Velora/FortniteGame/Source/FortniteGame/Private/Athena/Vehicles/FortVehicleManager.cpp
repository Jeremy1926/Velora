#include "pch.h"
#include "Athena/Vehicles/FortVehicleManager.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Items/FortVehicleItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CoreMiscDefines.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

struct _Script_FortniteGame_eventOnVehicleSpawnsUpdated_Parms
{
    UFortVehicleItemDefinition* VehicleItemDef;
    TArray<AActor*> SpawnedVehicles;
    int32 NewVehicleCount;
};

UClass* AFortVehicleManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleManager"));

    return Class;
}

bool AFortVehicleManager::GetVehicleManager(const UObject* WorldContextObject, AFortVehicleManager*& VehicleManager)
{
    static auto GetVehicleManager = Anchor::Optional<bool (*)(const UObject*, AFortVehicleManager**)>(TEXT("AFortVehicleManager::GetVehicleManager"))
        .Native(TEXT("/Script/FortniteGame.FortVehicleManager:GetVehicleManager"))
        .Get();

    VehicleManager = nullptr;

    return GetVehicleManager && GetVehicleManager(WorldContextObject, &VehicleManager) && VehicleManager;
}

void AFortVehicleManager::UpdateVehicleSpawns()
{
    auto OnVehicleSpawnsUpdated = GetOnVehicleSpawnsUpdated();

    TArray<AActor*> VehicleSpawners;
    UGameplayStatics::GetAllActorsOfClass(this, AFortAthenaVehicleSpawner::StaticClass(), VehicleSpawners);

    TArray<UFortVehicleItemDefinition*> VehicleItemDefs;
    TArray<AActor*> SpawnedVehicles;

    for (auto VehicleSpawner : VehicleSpawners)
    {
        ((AFortAthenaVehicleSpawner*)VehicleSpawner)->SpawnVehicle();

        auto SpawnedVehicle = OnVehicleSpawnsUpdated ? ((AFortAthenaVehicleSpawner*)VehicleSpawner)->GetSpawnedVehicle() : nullptr;
        auto VehicleItemDef = SpawnedVehicle ? ((AFortAthenaVehicleSpawner*)VehicleSpawner)->GetFortVehicleItemDef() : nullptr;

        if (!VehicleItemDef)
            continue;

        VehicleItemDefs.Add(VehicleItemDef);
        SpawnedVehicles.Add(SpawnedVehicle);
    }

    for (int32 Index = 0; Index < VehicleItemDefs.Num(); Index++)
    {
        int32 FirstIndex = INDEX_NONE;

        if (!VehicleItemDefs.Find(VehicleItemDefs[Index], FirstIndex) || FirstIndex != Index)
            continue;

        _Script_FortniteGame_eventOnVehicleSpawnsUpdated_Parms Parms{ VehicleItemDefs[Index] };

        for (int32 VehicleIndex = Index; VehicleIndex < VehicleItemDefs.Num(); VehicleIndex++)
        {
            if (VehicleItemDefs[VehicleIndex] == VehicleItemDefs[Index])
                Parms.SpawnedVehicles.Add(SpawnedVehicles[VehicleIndex]);
        }

        Parms.NewVehicleCount = Parms.SpawnedVehicles.Num();

        OnVehicleSpawnsUpdated->ProcessMulticastDelegate<UObject>(&Parms);
    }
}

FMulticastScriptDelegate* AFortVehicleManager::GetOnVehicleSpawnsUpdated()
{
    static auto OnVehicleSpawnsUpdated = Anchor::Offset(TEXT("AFortVehicleManager::OnVehicleSpawnsUpdated"))
        .String(TEXT("&UFortDagwoodVehicleManager::OnVehicleSpawnsUpdated"))
        .Optional()
        .FindEither("lea rcx, [*+0x*]; lea rdx, [*]; call 0x*", "add rdi, 0x*")
        .Get();

    return OnVehicleSpawnsUpdated > 0 ? (FMulticastScriptDelegate*)((uint8*)this + OnVehicleSpawnsUpdated) : nullptr;
}
