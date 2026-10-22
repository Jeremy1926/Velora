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
        .CallIn(Anchor::Address()
            .String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases."))
            .Start()
            .Get())
        .AtCall());

    Super::FinishWorldInitialization();
}

void AFortGameModeZone::CreateAIDirector()
{
    DETOUR(AFortGameModeZone, CreateAIDirector, Anchor::Hook()
        .String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"))
        .Optional()
        .Start()
        .Find("call 0x*; mov rax, [rbx+0x*]")
        .AtCall());

    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));
    auto AIDirectors = UFortGameData::Get()->GetAIDirectors();

    if (!AIDirectors)
        return;

    static auto GameGetAIDirector = Anchor::Data<const uint8_t*>(TEXT("UFortKismetLibrary::GetAIDirector"))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector"))
        .Get();

    static auto execGetAIDirector = THook(TEXT("UFortKismetLibrary::execGetAIDirector"), &UFortKismetLibrary::execGetAIDirector)
        .At(GameGetAIDirector)
        .Is("xor eax, eax; ret")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector"))
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector"))
        .Find("setnz *; add [*+0x20], *; and qword ptr [*], 0x0")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortKismetLibrary:GetAIDirector"));

    if (execGetAIDirector)
    {
        static auto StartMissionAIEncounter = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:StartMissionAIEncounter"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto StartMissionAIEncounterFromGeneratedProfile = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:StartMissionAIEncounterFromGeneratedProfile"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto StopMissionAIEncounter = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:StopMissionAIEncounter"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto CreateEncounterSequence = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:CreateEncounterSequence"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto CreateEncounterSequenceByIndex = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:CreateEncounterSequenceByIndex"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto StopEncounterSequence = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission:StopEncounterSequence"))
            .FirstCalleeCalling(GameGetAIDirector)
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto HandleAthenaSafeZonePhaseChanged = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .Exec(TEXT("/Script/FortniteGame.FortMission_RiftSpawners:HandleAthenaSafeZonePhaseChanged"))
            .Find("jmp 0x*")
            .Target()
            .Log(TEXT("AFortMission_RiftSpawners::HandleAthenaSafeZonePhaseChanged"))
            .Optional()
            .AtCallsToInCallees(GameGetAIDirector);

        static auto BindData = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .At(GameGetAIDirector)
            .VirtualCaller(TEXT("/Script/FortniteGame.Default__FortAIDataProvider_AIDirector"))
            .LogSlot(TEXT("UFortAIDataProvider_AIDirector::BindData"))
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto Update = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .At(GameGetAIDirector)
            .VirtualCaller(TEXT("/Script/FortniteGame.Default__AthenaAISense_Hearing"))
            .LogSlot(TEXT("UAthenaAISense_Hearing::Update"))
            .Optional()
            .AtCallsTo(GameGetAIDirector);

        static auto HandleDeathNotifications = THook(TEXT("UFortKismetLibrary::GetAIDirector"), &UFortKismetLibrary::GetAIDirector)
            .String(TEXT("Failed to find spawn group info on enemy pawn death for pawn %s. Associated Mission Name: %s. Group GUID is %s. Num active groups %i."))
            .Caller()
            .Optional()
            .AtCallsTo(GameGetAIDirector);
    }

    auto AIDirectorIndex = GetOverrideAIDirectorIndex();

    if (AIDirectorIndex < 0 || AIDirectorIndex >= AIDirectors->Num())
        return;

    auto AIDirectorClass = (*AIDirectors)[AIDirectorIndex].LoadSynchronous();
    static auto CanCreateInCurrentContext = THook(TEXT("UObject::CanCreateInCurrentContext"), &UObject::CanCreateInCurrentContext)
        .String(TEXT("Unable to spawn class '%s' due to client/server context."))
        .Last("call 0x*; test al, al")
        .AtCall();

    if (!CanCreateInCurrentContext)
        UObject::PatchCanCreateInCurrentContext();

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
    static auto CreateAIDirector = Anchor::VirtualFunction<void (*)(AFortGameModeZone*)>(TEXT("AFortGameModeZone::CreateAIDirector"))
        .String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"))
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get();

    if (!*AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this))
        CreateAIDirector(this);

    return *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this);
}

int32 AFortGameModeZone::GetOverrideAIDirectorIndex() const
{
    static auto OverrideAIDirectorIndex = GetClass()->FindPropertyByName(FName(TEXT("OverrideAIDirectorIndex")));

    return *OverrideAIDirectorIndex->ContainerPtrToValuePtr<int32>(this);
}

bool AFortGameModeZone::ShouldWeaponRemoveGrantedAbilities(AFortWeapon* Weapon, EFortWeaponAbilityRemovalReason Reason) const
{
    static auto ShouldWeaponRemoveGrantedAbilities = Anchor::VirtualFunction<bool (*)(const AFortGameModeZone*, AFortWeapon*, EFortWeaponAbilityRemovalReason)>(TEXT("AFortGameModeZone::ShouldWeaponRemoveGrantedAbilities"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "test rdx, rdx; jz *; cmp r8b, 0x2; jnz *; mov rcx, rdx; jmp 0x*")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"), "sub rsp, 0x28; mov al, 0x1; test rdx, rdx; jz *; cmp r8b, 0x2; jnz *; mov rcx, rdx; call 0x*")
        .Get();

    return ShouldWeaponRemoveGrantedAbilities(this, Weapon, Reason);
}

TArray<FItemAndCount>& AFortGameModeZone::GetStartingItems()
{
    static auto StartingItems = GetClass()->FindPropertyByName(FName(TEXT("StartingItems")));

    return *StartingItems->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}
