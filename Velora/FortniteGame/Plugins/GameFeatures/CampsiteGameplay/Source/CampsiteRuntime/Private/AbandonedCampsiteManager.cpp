#include "pch.h"
#include "AbandonedCampsiteManager.h"
#include "AbandonedCampsitePlacedSpawner.h"
#include "FortGameStateComponent_Campsite.h"
#include "FortControllerComponent_CampsiteAccountItem.h"
#include "Items/FortCampsiteAccountItem.h"
#include "Player/FortPlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Athena/FortGameStateAthena.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameplayEffectTypes.h"
#include "HAL/IConsoleManager.h"
#include "Engine/World.h"
#include "Containers/SparseArray.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/ScriptInterface.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Scanner.h"
#include "Detour.h"

static TWeakObjectPtr<AAbandonedCampsiteManager> SpawnedManager;
static TWeakObjectPtr<AAbandonedCampsiteManager> SpawningManager;
static TWeakObjectPtr<UEnvQueryInstanceBlueprintWrapper> SpawnQueryInstance;
static TArray<FVector> DeferredSpawnLocations;
static int32 NumQueryCampsites;
static int32 QueryTicksWaited;

UClass* AAbandonedCampsiteManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CampsiteRuntime.AbandonedCampsiteManager"));

    return Class;
}

void AAbandonedCampsiteManager::BeginPlay()
{
    DETOUR(AAbandonedCampsiteManager, BeginPlay, Anchor::Hook()
        .If(StaticClass())
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov rdx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::BeginPlay"))
        .InVTable(TEXT("/Script/CampsiteRuntime.Default__AbandonedCampsiteManager")));

    Original();

    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!HasAuthority() || !GameState)
        return;

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("OnGamePhaseStepChanged")));

    GameState->GetGamePhaseStepChanged().AddUnique(Delegate);
}

void AAbandonedCampsiteManager::execOnGamePhaseStepChanged(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AAbandonedCampsiteManager, execOnGamePhaseStepChanged, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/CampsiteRuntime.AbandonedCampsiteManager:OnGamePhaseStepChanged"))
        .Optional()
        .InExec(TEXT("/Script/CampsiteRuntime.AbandonedCampsiteManager:OnGamePhaseStepChanged")));

    FScriptInterface SafeZoneInterface;
    EAthenaGamePhaseStep GamePhaseStep{};

    Stack.StepCompiledIn(&SafeZoneInterface);
    Stack.StepCompiledIn(&GamePhaseStep);
    Stack.Code += !!Stack.Code;

    OnGamePhaseStepChanged(GamePhaseStep);
}

void AAbandonedCampsiteManager::OnGamePhaseStepChanged(EAthenaGamePhaseStep GamePhaseStep)
{
    static auto Warmup = (EAthenaGamePhaseStep)StaticEnum<EAthenaGamePhaseStep>()->GetValueByName(FName(TEXT("EAthenaGamePhaseStep::Warmup")));

    if (HasAuthority() && GamePhaseStep == Warmup && SpawnedManager.Get() != this)
        SpawnCampsites();
}

void AAbandonedCampsiteManager::SpawnCampsites()
{
    auto GameStateComponent = UFortGameStateComponent_Campsite::Get(this);

    if (!GameStateComponent || GetAbandonedCampsitesEnabled().GetValueAtLevel(0.0f) <= 0.0f)
        return;

    SpawnedManager = this;
    SpawningManager = this;

    auto& PreplacedCampsiteSpawnPoints = GetPreplacedCampsiteSpawnPoints();
    auto& DeferredPreplacedCampsiteSpawns = GetDeferredPreplacedCampsiteSpawns();

    PreplacedCampsiteSpawnPoints.Empty();

    for (auto PlacedCampsiteSpawnPoint : GameStateComponent->GetPlacedCampsiteSpawnPoints())
    {
        if (PlacedCampsiteSpawnPoint)
            PreplacedCampsiteSpawnPoints.Add(PlacedCampsiteSpawnPoint);
    }

    TArray<FVector> ProfileCampsiteLocations;

    for (auto PlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto PlayerController = PlayerState ? Cast<AFortPlayerController>(PlayerState->GetOwner()) : nullptr;
        auto AccountItemComponent = PlayerController ? PlayerController->FindComponentByClass<UFortControllerComponent_CampsiteAccountItem>() : nullptr;

        if (!AccountItemComponent)
            continue;

        auto& CampSpawnLocations = AccountItemComponent->GetCampsiteAccountData().GetCampSpawnLocations();

        for (int32 Index = 0; Index < CampSpawnLocations.Num(); Index++)
            ProfileCampsiteLocations.Add(CampSpawnLocations[Index].GetWorldSpawnLocation());
    }

    auto PlacedInLevelWeight = FindSpawnTypeWeight(EAbandonedCampsiteSpawnType::PlacedInLevel);
    auto FromPlayerProfileWeight = FindSpawnTypeWeight(EAbandonedCampsiteSpawnType::FromPlayerProfile);
    auto EnvironmentalQueryWeight = FindSpawnTypeWeight(EAbandonedCampsiteSpawnType::EnvironmentalQuery);
    const float PlacedWeight = PlacedInLevelWeight ? FMath::Max(PlacedInLevelWeight->GetValueAtLevel(0.0f), 0.0f) : 0.0f;
    const float ProfileWeight = FromPlayerProfileWeight ? FMath::Max(FromPlayerProfileWeight->GetValueAtLevel(0.0f), 0.0f) : 0.0f;
    const float QueryWeight = EnvironmentalQueryWeight && GetSpawnEnvironmentalQuery() ? FMath::Max(EnvironmentalQueryWeight->GetValueAtLevel(0.0f), 0.0f) : 0.0f;
    const int32 NumCampsites = FMath::RandRange(FMath::RoundToInt(GetMinNumAbandonedCampsites().GetValueAtLevel(0.0f)), FMath::RoundToInt(GetMaxNumAbandonedCampsites().GetValueAtLevel(0.0f)));
    int32 NumEnvironmentalQueryCampsites = 0;
    int32 NumProfileCampsites = 0;

    for (int32 Index = 0; Index < NumCampsites; Index++)
    {
        const float AvailablePlacedWeight = PreplacedCampsiteSpawnPoints.Num() ? PlacedWeight : 0.0f;
        const float AvailableProfileWeight = ProfileCampsiteLocations.Num() > NumProfileCampsites ? ProfileWeight : 0.0f;
        const float TotalWeight = AvailablePlacedWeight + AvailableProfileWeight + QueryWeight;

        if (TotalWeight <= 0.0f)
            break;

        const float Roll = FMath::FRand() * TotalWeight;

        if (Roll >= AvailablePlacedWeight + AvailableProfileWeight)
        {
            NumEnvironmentalQueryCampsites++;
            continue;
        }

        if (Roll >= AvailablePlacedWeight)
        {
            NumProfileCampsites++;
            continue;
        }

        const int32 SpawnPointIndex = FMath::RandHelper(PreplacedCampsiteSpawnPoints.Num());

        DeferredPreplacedCampsiteSpawns.Add(PreplacedCampsiteSpawnPoints[SpawnPointIndex]);
        PreplacedCampsiteSpawnPoints.RemoveAt(SpawnPointIndex);
    }

    StartEnvironmentalQuerySpawns(NumEnvironmentalQueryCampsites);

    for (; NumProfileCampsites > 0; NumProfileCampsites--)
        SpawnRandomCampsiteFromProfiles(ProfileCampsiteLocations);
}

void AAbandonedCampsiteManager::SpawnRandomCampsiteFromProfiles(TArray<FVector>& ProfileCampsiteLocations)
{
    static auto MinimumDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("Campsite.MinimumDistance"));

    const float MinimumDistanceValue = MinimumDistance ? MinimumDistance->GetFloat() : 0.0f;

    while (ProfileCampsiteLocations.Num())
    {
        const int32 LocationIndex = FMath::RandHelper(ProfileCampsiteLocations.Num());
        const FVector Location = ProfileCampsiteLocations[LocationIndex];

        ProfileCampsiteLocations.RemoveAt(LocationIndex);

        if (IsFarEnoughFromSpawns(Location, MinimumDistanceValue * MinimumDistanceValue))
        {
            DeferredSpawnLocations.Add(Location);
            return;
        }
    }
}

void AAbandonedCampsiteManager::StartEnvironmentalQuerySpawns(int32 NumCampsites)
{
    NumQueryCampsites = NumCampsites;
    QueryTicksWaited = 0;
    DeferredSpawnLocations.Empty();
    SpawnQueryInstance = NumCampsites > 0 ? UEnvQueryManager::RunEQSQuery(this, GetSpawnEnvironmentalQuery(), this, EEnvQueryRunMode::AllMatching, nullptr) : nullptr;

    if (!SpawnQueryInstance.Get())
        NumQueryCampsites = 0;
}

void AAbandonedCampsiteManager::DeferredCampsiteSpawnLoop()
{
    static auto NumCampsitesToSpawnPerFrame = IConsoleManager::Get().FindConsoleVariable(TEXT("Campsite.NumCampsitesToSpawnPerFrame"));

    auto& DeferredPreplacedCampsiteSpawns = GetDeferredPreplacedCampsiteSpawns();

    for (int32 Count = FMath::Min(NumCampsitesToSpawnPerFrame ? NumCampsitesToSpawnPerFrame->GetInt() : 0, DeferredPreplacedCampsiteSpawns.Num() + DeferredSpawnLocations.Num()); Count >= 0; Count--)
    {
        if (DeferredPreplacedCampsiteSpawns.Num() > 0)
        {
            auto PreplacedCampsiteSpawn = DeferredPreplacedCampsiteSpawns[DeferredPreplacedCampsiteSpawns.Num() - 1];

            DeferredPreplacedCampsiteSpawns.RemoveAt(DeferredPreplacedCampsiteSpawns.Num() - 1);

            if (PreplacedCampsiteSpawn)
                PreplacedCampsiteSpawn->SpawnCampsite();
        }
        else if (DeferredSpawnLocations.Num() > 0)
        {
            const FVector SpawnLocation = DeferredSpawnLocations[DeferredSpawnLocations.Num() - 1];

            DeferredSpawnLocations.RemoveAt(DeferredSpawnLocations.Num() - 1);

            if (auto AbandonedCampsiteAsset = GetAbandonedCampsiteAsset())
                GetWorld()->SpawnActor(AbandonedCampsiteAsset, &SpawnLocation);
        }
    }
}

bool AAbandonedCampsiteManager::IsFarEnoughFromSpawns(const FVector& Location, float MinimumDistanceSquared)
{
    for (auto PreplacedCampsiteSpawn : GetDeferredPreplacedCampsiteSpawns())
    {
        if (PreplacedCampsiteSpawn && (PreplacedCampsiteSpawn->GetActorLocation() - Location).SizeSquared() < MinimumDistanceSquared)
            return false;
    }

    for (auto& SpawnLocation : DeferredSpawnLocations)
    {
        if ((SpawnLocation - Location).SizeSquared() < MinimumDistanceSquared)
            return false;
    }

    return true;
}

void AAbandonedCampsiteManager::TickDeferredCampsiteSpawns()
{
    auto Manager = SpawningManager.Get();

    if (!Manager)
        return;

    if (NumQueryCampsites > 0)
    {
        auto QueryInstance = SpawnQueryInstance.Get();
        auto Locations = QueryInstance ? QueryInstance->GetResultsAsLocations() : TArray<FVector>();

        if (!Locations.Num() && QueryInstance && ++QueryTicksWaited < 300)
            return;

        static auto MinimumDistance = IConsoleManager::Get().FindConsoleVariable(TEXT("Campsite.MinimumDistance"));
        const float MinimumDistanceValue = MinimumDistance ? MinimumDistance->GetFloat() : 0.0f;

        while (NumQueryCampsites > 0 && Locations.Num())
        {
            const int32 LocationIndex = FMath::RandHelper(Locations.Num());
            const FVector Location = Locations[LocationIndex];

            Locations.RemoveAt(LocationIndex);

            if (!Manager->IsFarEnoughFromSpawns(Location, MinimumDistanceValue * MinimumDistanceValue))
                continue;

            DeferredSpawnLocations.Add(Location);
            NumQueryCampsites--;
        }

        NumQueryCampsites = 0;
        SpawnQueryInstance = nullptr;
    }

    Manager->DeferredCampsiteSpawnLoop();

    if (!Manager->GetDeferredPreplacedCampsiteSpawns().Num() && !DeferredSpawnLocations.Num())
        SpawningManager = nullptr;
}

FScalableFloat& AAbandonedCampsiteManager::GetMinNumAbandonedCampsites()
{
    static auto MinNumAbandonedCampsites = StaticClass()->FindPropertyByName(FName(TEXT("MinNumAbandonedCampsites")));

    return *MinNumAbandonedCampsites->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& AAbandonedCampsiteManager::GetMaxNumAbandonedCampsites()
{
    static auto MaxNumAbandonedCampsites = StaticClass()->FindPropertyByName(FName(TEXT("MaxNumAbandonedCampsites")));

    return *MaxNumAbandonedCampsites->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FScalableFloat& AAbandonedCampsiteManager::GetAbandonedCampsitesEnabled()
{
    static auto AbandonedCampsitesEnabled = StaticClass()->FindPropertyByName(FName(TEXT("AbandonedCampsitesEnabled")));

    return *AbandonedCampsitesEnabled->ContainerPtrToValuePtr<FScalableFloat>(this);
}

const FScalableFloat* AAbandonedCampsiteManager::FindSpawnTypeWeight(EAbandonedCampsiteSpawnType SpawnType)
{
    static auto SpawnTypeWeights = StaticClass()->FindPropertyByName(FName(TEXT("SpawnTypeWeights")));
    static auto ValueAlignment = FScalableFloat::StaticStruct()->GetMinAlignment();
    static auto ElementAlignment = FMath::Max(ValueAlignment, (int32)alignof(int32));
    static auto ValueOffset = ((int32)sizeof(EAbandonedCampsiteSpawnType) + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto PairSize = (ValueOffset + FScalableFloat::StaticStruct()->GetPropertiesSize() + ValueAlignment - 1) & ~(ValueAlignment - 1);
    static auto ElementSize = (PairSize + (int32)sizeof(int32) * 2 + ElementAlignment - 1) & ~(ElementAlignment - 1);

    auto& Elements = *SpawnTypeWeights->ContainerPtrToValuePtr<TSparseArray<uint8>>(this);

    for (int32 Index = 0; Index < Elements.GetMaxIndex(); Index++)
    {
        auto Element = (const uint8*)&Elements[0] + Index * ElementSize;

        if (Elements.IsAllocated(Index) && *(const EAbandonedCampsiteSpawnType*)Element == SpawnType)
            return (const FScalableFloat*)(Element + ValueOffset);
    }

    return nullptr;
}

UEnvQuery* AAbandonedCampsiteManager::GetSpawnEnvironmentalQuery()
{
    static auto Spawn_EnvironmentalQuery = StaticClass()->FindPropertyByName(FName(TEXT("Spawn_EnvironmentalQuery")));

    return Spawn_EnvironmentalQuery->ContainerPtrToValuePtr<TSoftObjectPtr<UEnvQuery>>(this)->LoadSynchronous();
}

UClass* AAbandonedCampsiteManager::GetAbandonedCampsiteAsset()
{
    static auto AbandonedCampsiteAsset = StaticClass()->FindPropertyByName(FName(TEXT("AbandonedCampsiteAsset")));

    return AbandonedCampsiteAsset->ContainerPtrToValuePtr<TSoftClassPtr<UObject>>(this)->LoadSynchronous();
}

TArray<AAbandonedCampsitePlacedSpawner*>& AAbandonedCampsiteManager::GetPreplacedCampsiteSpawnPoints()
{
    static auto PreplacedCampsiteSpawnPoints = StaticClass()->FindPropertyByName(FName(TEXT("PreplacedCampsiteSpawnPoints")));

    return *PreplacedCampsiteSpawnPoints->ContainerPtrToValuePtr<TArray<AAbandonedCampsitePlacedSpawner*>>(this);
}

TArray<AAbandonedCampsitePlacedSpawner*>& AAbandonedCampsiteManager::GetDeferredPreplacedCampsiteSpawns()
{
    static auto DeferredPreplacedCampsiteSpawns = StaticClass()->FindPropertyByName(FName(TEXT("DeferredPreplacedCampsiteSpawns")));

    return *DeferredPreplacedCampsiteSpawns->ContainerPtrToValuePtr<TArray<AAbandonedCampsitePlacedSpawner*>>(this);
}
