#include "pch.h"
#include "Athena/FortAthenaAircraft.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortGameStateAthena.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FAircraftFlightInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AircraftFlightInfo"));

    return Struct;
}

int32 FAircraftFlightInfo::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

UClass* AFortAthenaAircraft::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAircraft"));

    return Class;
}

AFortAthenaAircraft* AFortAthenaAircraft::SpawnAircraft(UWorld* World, int32 AircraftIndex)
{
    static auto SpawnAircraft = Anchor::Function<AFortAthenaAircraft* (*)(UWorld*, UClass*, const FAircraftFlightInfo&, UObject*)>(TEXT("AFortAthenaAircraft::SpawnAircraft"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAircraft:SpawnAircraft"))
        .Find("add [*+0x20], *; cmp *")
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAircraft:SpawnAircraft"))
        .Find("setnz *; add *, *; mov [*+0x20], *; call 0x*")
        .Call()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortAthenaAircraft:SpawnAircraft"))
        .Get();

    static auto bClassByAddress = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAircraft:SpawnAircraft"))
        .Find("add [*+0x20], *; cmp *")
        .Get();

    auto GameState = Cast<AFortGameStateAthena>(World->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;

    if (!MapInfo || !MapInfo->IsMapInfoValidForAircraft())
        return nullptr;

    auto AircraftClass = MapInfo->GetAircraftClass();
    auto& FlightInfos = MapInfo->GetFlightInfos();
    auto& FlightInfo = AircraftIndex >= 0 && AircraftIndex < FlightInfos.Num() ? FlightInfos[AircraftIndex] : ((AFortAthenaAircraft*)AircraftClass->GetDefaultObject())->GetFlightInfo();
    auto Aircraft = bClassByAddress ? ((AFortAthenaAircraft* (*)(UWorld*, UClass* const*, const FAircraftFlightInfo&, UObject*))SpawnAircraft)(World, &AircraftClass, FlightInfo, nullptr) : SpawnAircraft(World, AircraftClass, FlightInfo, nullptr);

    if (Aircraft)
        Aircraft->GetAircraftIndex() = AircraftIndex;

    return Aircraft;
}

const FAircraftFlightInfo& AFortAthenaAircraft::GetFlightInfo() const
{
    static auto FlightInfo = StaticClass()->FindPropertyByName(FName(TEXT("FlightInfo")));

    return *FlightInfo->ContainerPtrToValuePtr<FAircraftFlightInfo>(this);
}

int32& AFortAthenaAircraft::GetAircraftIndex()
{
    static auto AircraftIndex = StaticClass()->FindPropertyByName(FName(TEXT("AircraftIndex")));

    return *AircraftIndex->ContainerPtrToValuePtr<int32>(this);
}
