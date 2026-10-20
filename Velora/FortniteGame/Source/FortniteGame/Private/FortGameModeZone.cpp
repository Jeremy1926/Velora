#include "pch.h"
#include "FortGameModeZone.h"
#include "FortGameData.h"
#include "AI/FortAIDirector.h"
#include "Athena/AI/AthenaAIDirector.h"
#include "Items/FortInventory.h"
#include "Engine/World.h"
#include "Engine/CurveTable.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto FinishWorldInitialization = []
{
    auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get();
    auto AthenaVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
    auto Slot = Scanner::Slot(TEXT("AFortGameMode::FinishWorldInitialization"), AthenaVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaFinishWorldInitialization; });
    auto PvPBaseFinishWorldInitialization = Scanner::Virtual<const void*>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGamePvPBase")), Slot);
    auto Call = Scanner::At(PvPBaseFinishWorldInitialization).Find("call 0x*");

    if (!Call.Get())
        Call = Scanner::At(AthenaFinishWorldInitialization).Find(std::format("call 0x{:x}", (uintptr_t)PvPBaseFinishWorldInitialization));

    return DetourCall(TEXT("AFortGameModeZone::FinishWorldInitialization"), Call.Get(), &AFortGameModeZone::FinishWorldInitialization);
}();

static auto CreateAIDirector = []
{
    auto AthenaCreateAIDirector = Scanner::String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"));

    return AthenaCreateAIDirector.Get() ? DetourCall(TEXT("AFortGameModeZone::CreateAIDirector"), AthenaCreateAIDirector.Start().Find("call 0x*").Get(), &AFortGameModeZone::CreateAIDirector) : nullptr;
}();

void AFortGameModeZone::FinishWorldInitialization()
{
    Super::FinishWorldInitialization();
}

void AFortGameModeZone::CreateAIDirector()
{
    static auto CanCreateInCurrentContext = DetourCall(TEXT("UObject::CanCreateInCurrentContext"), Scanner::String(TEXT("Unable to spawn class '%s' due to client/server context.")).Last("call 0x*; test al, al").Get(), &UObject::CanCreateInCurrentContext);
    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));

    auto& AIDirectors = UFortGameData::Get()->GetAIDirectors();
    auto AIDirectorIndex = GetOverrideAIDirectorIndex();

    if (AIDirectorIndex < 0 || AIDirectorIndex >= AIDirectors.Num())
        return;

    auto AIDirectorClass = AIDirectors[AIDirectorIndex].LoadSynchronous();
    auto NewAIDirector = (AFortAIDirector*)GetWorld()->SpawnActor(AIDirectorClass ? AIDirectorClass : AAthenaAIDirector::StaticClass());

    *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this) = NewAIDirector;

    if (!NewAIDirector || AIDirectorClass || AIDirectors[AIDirectorIndex].GetLongPackageName() != FString(TEXT("/Game/Athena/Deimos/AIDirector/Deimos_AIDirector")))
        return;

    NewAIDirector->GetBaseEncounterClass() = LoadObject<UClass>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/Encounters/Deimos_BaseEncounterSettings.Deimos_BaseEncounterSettings_C"));
    NewAIDirector->GetDummyDebugEncounter() = LoadObject<UClass>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/Encounters/Deimos_DummyDebugEncounter.Deimos_DummyDebugEncounter_C"));

    if (auto BaseLootDrops = LoadObject<UDataTable>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/DataTables/Deimos_BaseLootDrops.Deimos_BaseLootDrops")))
    {
        for (auto& RowPair : BaseLootDrops->RowMap)
            NewAIDirector->GetBaseLootDropData().Add(FDataTableRowHandle{ BaseLootDrops, RowPair.Key });
    }

    auto SpawnBudgetCurves = LoadObject<UCurveTable>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/DataTables/Deimos_AIDirectorSpawnBudgetCurves.Deimos_AIDirectorSpawnBudgetCurves"));
    auto PopulationCategoryCaps = LoadObject<UCurveTable>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/DataTables/Deimos_AIDirectorPopulationCategoryCaps.Deimos_AIDirectorPopulationCategoryCaps"));

    for (auto EncounterType : { TEXT("ActiveThreat"), TEXT("AmbientThreat") })
    {
        auto EncounterTypeTag = FGameplayTag::RequestGameplayTag(FName(std::format(TEXT("Encounter.Type.PvP.Deimos.{}"), EncounterType).c_str()));

        auto SpawnPointsProfile = new(NewAIDirector->GetEncounterSpawnPointsSettings()) FFortAIEncounterSpawnPointsProfile();

        SpawnPointsProfile->GetEncounterTypeTags().AddTag(EncounterTypeTag);
        SpawnPointsProfile->GetMaxSpawnPointsPerPlayerCount().Add(FCurveTableRowHandle{ SpawnBudgetCurves, FName(std::format(TEXT("PvP_Deimos_{}_All_MaxSpawnPoints"), EncounterType).c_str()) });
        SpawnPointsProfile->GetMinSpawnPointsPerPlayerCount().Add(FCurveTableRowHandle{ SpawnBudgetCurves, FName(std::format(TEXT("PvP_Deimos_{}_All_MinSpawnPoints"), EncounterType).c_str()) });

        auto SpawnGroupCapsProfile = new(NewAIDirector->GetEncounterSpawnGroupCapSettings()) FFortAIEncounterSpawnGroupCapsProfile();

        SpawnGroupCapsProfile->GetEncounterTypeTags().AddTag(EncounterTypeTag);

        for (auto PopulationCategory : { TEXT("Basic"), TEXT("Common"), TEXT("Uncommon"), TEXT("Rare") })
        {
            auto SpawnGroupCapsCategory = new(SpawnGroupCapsProfile->GetPopulationCategories()) FFortAIEncounterSpawnGroupCapsCategory();
            auto SpawnGroupCap = new(SpawnGroupCapsCategory->GetSpawnGroupCapsPerPlayerCount()) FFortAIEncounterSpawnGroupCap();

            SpawnGroupCapsCategory->GetTagQuery() = FGameplayTagQuery::MakeQuery_MatchTag(FGameplayTag::RequestGameplayTag(FName(std::format(TEXT("Encounter.PopulationCategory.{}"), PopulationCategory).c_str())));
            SpawnGroupCapsCategory->GetInitialSpawnGroupAvailabilityDelaySeconds() = FCurveTableRowHandle{ PopulationCategoryCaps, FName(std::format(TEXT("PvP_Deimos_{}_{}_AllPlayers_InitialDelay"), EncounterType, PopulationCategory).c_str()) };
            SpawnGroupCapsCategory->GetSpawnGroupAvailabilityDelaySeconds() = FCurveTableRowHandle{ PopulationCategoryCaps, FName(std::format(TEXT("PvP_Deimos_{}_{}_AllPlayers_RespawnTime"), EncounterType, PopulationCategory).c_str()) };
            SpawnGroupCapsCategory->GetInitialSpawnGroupAvailabilityTime() = -1.0f;
            SpawnGroupCap->GetMinSpawnGroupNumberCap() = FCurveTableRowHandle{ PopulationCategoryCaps, FName(std::format(TEXT("PvP_Deimos_{}_{}_AllPlayers_Min"), EncounterType, PopulationCategory).c_str()) };
            SpawnGroupCap->GetMaxSpawnGroupNumberCap() = FCurveTableRowHandle{ PopulationCategoryCaps, FName(std::format(TEXT("PvP_Deimos_{}_{}_AllPlayers_Max"), EncounterType, PopulationCategory).c_str()) };
        }
    }
}

AFortAIDirector* AFortGameModeZone::GetAIDirector()
{
    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));
    static auto CreateAIDirector = []
    {
        auto AthenaCreateAIDirector = Scanner::String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i")).Start().Get();
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));

        return Scanner::Slot(TEXT("AFortGameModeZone::CreateAIDirector"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaCreateAIDirector; });
    }();

    if (!*AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this))
        Scanner::Virtual<void (*)(AFortGameModeZone*)>(this, CreateAIDirector)(this);

    return *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this);
}

int32 AFortGameModeZone::GetOverrideAIDirectorIndex() const
{
    static auto OverrideAIDirectorIndex = GetClass()->FindPropertyByName(FName(TEXT("OverrideAIDirectorIndex")));

    return *OverrideAIDirectorIndex->ContainerPtrToValuePtr<int32>(this);
}

TArray<FItemAndCount>& AFortGameModeZone::GetStartingItems()
{
    static auto StartingItems = GetClass()->FindPropertyByName(FName(TEXT("StartingItems")));

    return *StartingItems->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}
