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
#include "EnvironmentQuery/EnvQuery.h"
#include "FortKismetLibrary.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortGameModeZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameModeZone"));

    return Class;
}

void AFortGameModeZone::FinishWorldInitialization()
{
    DETOUR(AFortGameModeZone, FinishWorldInitialization, Anchor::Hook()
        .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .LogSlot(TEXT("AFortGameMode::FinishWorldInitialization"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortGamePvPBase"))
        .Find("call 0x*")
        .Or()
        .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortGamePvPBase"))
        .CallIn(Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start())
        .AtCall());

    Super::FinishWorldInitialization();
}

void AFortGameModeZone::CreateAIDirector()
{
    DETOUR(AFortGameModeZone, CreateAIDirector, Anchor::Hook()
        .String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"))
        .Optional()
        .Start()
        .Find("call 0x*")
        .AtCall());

    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));
    auto AIDirectors = UFortGameData::Get()->GetAIDirectors();

    if (!AIDirectors)
        return;

    static auto GetAIDirector = []
    {
        auto GameGetAIDirector = Scanner::Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector")).Get(TEXT("UFortKismetLibrary::GetAIDirector"));
        auto CallGetAIDirector = std::format("call 0x{:x}", (uintptr_t)GameGetAIDirector);
        auto execGetAIDirector = THook(TEXT("UFortKismetLibrary::execGetAIDirector"), &UFortKismetLibrary::execGetAIDirector)
            .At(GameGetAIDirector)
            .Is("xor eax, eax; ret")
            .Optional()
            .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector"));

        if (!execGetAIDirector)
            return false;

        auto RedirectGetAIDirector = [&](const uint8_t* Caller)
        {
            THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
                .At(GameGetAIDirector)
                .AtCallsIn(Scanner::At(Caller));
        };

        for (auto FunctionPath : { TEXT("/Script/FortniteGame.FortMission:StartMissionAIEncounter"), TEXT("/Script/FortniteGame.FortMission:StartMissionAIEncounterFromGeneratedProfile"), TEXT("/Script/FortniteGame.FortMission:StopMissionAIEncounter"), TEXT("/Script/FortniteGame.FortMission:CreateEncounterSequence"), TEXT("/Script/FortniteGame.FortMission:CreateEncounterSequenceByIndex"), TEXT("/Script/FortniteGame.FortMission:StopEncounterSequence") })
        {
            for (auto Call = Scanner::Exec(FunctionPath).Find("call 0x*"); Call.Get(); Call = Call.Find("call 0x*"))
            {
                if (Call.Target() != GameGetAIDirector && Scanner::At(Call.Target()).Has(CallGetAIDirector))
                {
                    RedirectGetAIDirector(Call.Target());
                    break;
                }
            }
        }

        auto HandleAthenaSafeZonePhaseChanged = Scanner::Exec(TEXT("/Script/FortniteGame.FortMission_RiftSpawners:HandleAthenaSafeZonePhaseChanged")).Find("jmp 0x*").Target(TEXT("AFortMission_RiftSpawners::HandleAthenaSafeZonePhaseChanged"));

        for (auto Call = Scanner::At(HandleAthenaSafeZonePhaseChanged).Find("call 0x*"); Call.Get(); Call = Call.Find("call 0x*"))
        {
            if (Call.Target() != GameGetAIDirector && Scanner::At(Call.Target()).Has(CallGetAIDirector))
                RedirectGetAIDirector(Call.Target());
        }

        for (auto [DefaultObjectPath, FunctionName] : { std::pair{ TEXT("/Script/FortniteGame.Default__FortAIDataProvider_AIDirector"), TEXT("UFortAIDataProvider_AIDirector::BindData") }, std::pair{ TEXT("/Script/FortniteGame.Default__AthenaAISense_Hearing"), TEXT("UAthenaAISense_Hearing::Update") } })
        {
            auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, DefaultObjectPath);
            auto Slot = Scanner::Slot(FunctionName, VTable, [&](const Scanner::FCode& Entry) { return Entry.Has(CallGetAIDirector); });

            if (Scanner::IsSlot(Slot))
                RedirectGetAIDirector((const uint8_t*)VTable[Slot / sizeof(void*)]);
        }

        RedirectGetAIDirector(Scanner::CallTo(Scanner::String(TEXT("Failed to find spawn group info on enemy pawn death for pawn %s. Associated Mission Name: %s. Group GUID is %s. Num active groups %i.")).Start().Get()).Start().Get());

        return true;
    }();

    auto AIDirectorIndex = GetOverrideAIDirectorIndex();

    if (AIDirectorIndex < 0 || AIDirectorIndex >= AIDirectors->Num())
        return;

    auto AIDirectorClass = (*AIDirectors)[AIDirectorIndex].LoadSynchronous();
    static auto CanCreateInCurrentContext = THook(TEXT("UObject::CanCreateInCurrentContext"), &UObject::CanCreateInCurrentContext)
        .String(TEXT("Unable to spawn class '%s' due to client/server context."))
        .Last("call 0x*; test al, al")
        .AtCall();
    auto NewAIDirector = (AFortAIDirector*)GetWorld()->SpawnActor(AIDirectorClass ? AIDirectorClass : AAthenaAIDirector::StaticClass());

    *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this) = NewAIDirector;

    if (!NewAIDirector || AIDirectorClass || (*AIDirectors)[AIDirectorIndex].GetLongPackageName() != FString(TEXT("/Game/Athena/Deimos/AIDirector/Deimos_AIDirector")))
        return;

    NewAIDirector->GetBaseEncounterClass() = LoadObject<UClass>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/Encounters/Deimos_BaseEncounterSettings.Deimos_BaseEncounterSettings_C"));
    NewAIDirector->GetDummyDebugEncounter() = LoadObject<UClass>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/Encounters/Deimos_DummyDebugEncounter.Deimos_DummyDebugEncounter_C"));
    NewAIDirector->GetSpawnLocationActorSearchQueries((EFortEncounterSpawnLocationPlacementMode)StaticEnum<EFortEncounterSpawnLocationPlacementMode>()->GetValueByName(FName(TEXT("EFortEncounterSpawnLocationPlacementMode::Custom")))).GetEnvironmentQuery() = LoadObject<UEnvQuery>(nullptr, TEXT("/Game/Athena/Deimos/AIDirector/EQS/Deimos_EQS_RiftSpawner_Search.Deimos_EQS_RiftSpawner_Search"));

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
