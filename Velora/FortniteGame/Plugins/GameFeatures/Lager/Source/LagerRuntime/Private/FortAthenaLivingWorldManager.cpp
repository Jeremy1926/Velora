#include "pch.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldCategory.h"
#include "FortAthenaLivingWorldConfigData.h"
#include "FortAthenaLivingWorldDensityGrid.h"
#include "FortAthenaLivingWorldEvent.h"
#include "FortAthenaLivingWorldEventData.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "Athena/AI/AthenaAISpawner.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/AI/FortAthenaAISpawnerData.h"
#include "Athena/FortGameStateAthena.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Templates/Casts.h"
#include "Templates/Sorting.h"
#include "UObject/Class.h"
#include "UObject/SparseDelegate.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeDataEntry> UFortAthenaLivingWorldManager::EventRuntimeData;

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventRequest> UFortAthenaLivingWorldManager::PendingEventRequests;

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingEQSSpawn> UFortAthenaLivingWorldManager::PendingEQSSpawns;

int32 UFortAthenaLivingWorldManager::NextEQSSpawnRequestID = INDEX_NONE;

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetTypeSize()
{
    return GetRespawnTimeOffset() + sizeof(float) + sizeof(int32);
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetRespawnTimeOffset()
{
    static auto RespawnTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::RespawnTime"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("addss xmm*, [*+0x*]; movss [*+0x*], xmm*")
        .Find("movss [*+0x*], xmm*")
        .Disp();

    return RespawnTime;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorsOffset()
{
    if (GetSpawnedActorPointProvidersOffset() <= 0)
        return GetRespawnTimeOffset() - (int32)sizeof(TArray<TWeakObjectPtr<AActor>>);

    static auto SpawnedActors = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::SpawnedActors"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov r9b, 0x1; lea rcx, [rdi+0x*]; mov edx, ebp; call 0x*")
        .Find("lea rcx, [rdi+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov r9b, 0x1; mov edx, r15d; call 0x*")
        .Disp();

    return SpawnedActors;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorPointProvidersOffset()
{
    static auto SpawnedActorPointProviders = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::FLivingWorldEventInstance::SpawnedActorPointProviders"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActorDestroyed")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("mov r14d, ebp; lea r15, [rdi+0x*]")
        .Find("lea r15, [rdi+0x*]")
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActorDestroyed")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("mov r14d, r15d; lea r12, [rbx+0x*]")
        .Find("lea r12, [rbx+0x*]")
        .Disp();

    return SpawnedActorPointProviders;
}

int32 UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetPendingSpawnRequestsOffset()
{
    static auto PendingSpawnRequests = GetSpawnedActorPointProvidersOffset() > 0 ? GetSpawnedActorPointProvidersOffset() + (int32)sizeof(TArray<FScriptInterface>) : (int32)(sizeof(FFortAthenaLivingWorldCategory*) + sizeof(FFortAthenaLivingWorldEvent*) + sizeof(TWeakObjectPtr<UFortAthenaLivingWorldEventData>));

    return PendingSpawnRequests;
}

UFortAthenaLivingWorldManager::FLivingWorldEventInstance::~FLivingWorldEventInstance()
{
    GetSpawnedActors().~TArray();

    if (GetPendingActorSpawnEventRequestsOffset() <= 0)
        GetPendingSpawnRequests().~TArray();

    if (auto SpawnedActorPointProviders = GetSpawnedActorPointProviders())
        SpawnedActorPointProviders->~TArray();
}

TArray<TWeakObjectPtr<AActor>>& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActors()
{
    return *(TArray<TWeakObjectPtr<AActor>>*)((uint8*)this + GetSpawnedActorsOffset());
}

TArray<FScriptInterface>* UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetSpawnedActorPointProviders()
{
    return GetSpawnedActorPointProvidersOffset() > 0 ? (TArray<FScriptInterface>*)((uint8*)this + GetSpawnedActorPointProvidersOffset()) : nullptr;
}

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingSpawnRequest>& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetPendingSpawnRequests()
{
    return *(TArray<FLivingWorldPendingSpawnRequest>*)((uint8*)this + GetPendingSpawnRequestsOffset());
}

float& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetRespawnTime()
{
    return *(float*)((uint8*)this + GetRespawnTimeOffset());
}

int32& UFortAthenaLivingWorldManager::FLivingWorldEventInstance::GetActorCount()
{
    return *(int32*)((uint8*)this + GetRespawnTimeOffset() + sizeof(float));
}

UClass* UFortAthenaLivingWorldManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager"));

    return Class;
}

int32 UFortAthenaLivingWorldManager::GetEventInstancesOffset()
{
    static auto EventInstances = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventInstances"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("movzx eax, byte ptr [rax+0x*]; xor r14d, r14d; mov [rsp+0x*], eax; cmp [*+0x*], r14d")
        .Find("lea r12, [*+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("xor r14d, r14d; test eax, eax; jle 0x*; lea *, [*+0x*]")
        .Find("lea *, [*+0x*]")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("xor r13d, r13d; test eax, eax; jle 0x*; lea *, [*+0x*]")
        .Find("lea *, [*+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("test ecx, ecx; jle 0x*; lea r14, [rsi+0x*]")
        .Find("lea r14, [rsi+0x*]")
        .Or()
        .If(StaticClass() && GetPendingActorSpawnEventRequestsOffset() <= 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("test ecx, ecx; jle 0x*; lea rsi, [rbp+0x*]")
        .Find("lea rsi, [rbp+0x*]")
        .Disp();

    return EventInstances;
}

int32 UFortAthenaLivingWorldManager::GetPendingActorSpawnEventRequestsOffset()
{
    static auto PendingActorSpawnEventRequests = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::PendingActorSpawnEventRequests"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Optional()
        .Find("test ecx, ecx; jle 0x*; lea r12, [r13+0x*]")
        .Find("lea r12, [r13+0x*]")
        .Disp();

    return PendingActorSpawnEventRequests;
}

void UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(AActor* PointProvider)
{
    if (GetEventInstancesOffset() <= 0)
        return;

    auto World = PointProvider ? PointProvider->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;

    if (!LivingWorldManager || !LivingWorldManager->IsLivingWorldEnabled() || !GameState->HasAuthority())
        return;

    LivingWorldManager->AddPointProvider(PointProvider);
}

void UFortAthenaLivingWorldManager::LivingWorldManagerUnregisterPointProvider(AActor* PointProvider)
{
    if (GetEventInstancesOffset() <= 0)
        return;

    auto World = PointProvider ? PointProvider->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;

    if (!LivingWorldManager || !GameState->HasAuthority())
        return;

    LivingWorldManager->RemovePointProvider(PointProvider);
}

bool UFortAthenaLivingWorldManager::IsLivingWorldEnabled()
{
    return GetLagerEnabled().GetValueAtLevel(0.0f) > 0.0f && !FParse::Param(FCommandLine::Get(), TEXT("NoLager"));
}

void UFortAthenaLivingWorldManager::TickComponent(float DeltaTime)
{
    auto World = GetWorld();
    auto& NextEventGenerationTime = GetNextEventGenerationTime();
    auto Config = GetConfig();

    if (!World || !Config || NextEventGenerationTime < 0.0f)
        return;

    if (World->GetTimeSeconds() >= NextEventGenerationTime)
    {
        NextEventGenerationTime = World->GetTimeSeconds() + Config->GetMinimumEventGenerationInterval().GetValueAtLevel(0.0f);

        GenerateEvents();
    }

    ProcessEventRequests();
    ProcessEQSRequests();

    auto ActorDensityGrid = GetActorDensityGrid();
    auto MaxActorDensityUpdatesPerTick = Config->GetMaxActorDensityUpdatesPerTick();

    if (ActorDensityGrid && MaxActorDensityUpdatesPerTick)
        ActorDensityGrid->UpdateActors((int32)MaxActorDensityUpdatesPerTick->GetValueAtLevel(0.0f));
}

void UFortAthenaLivingWorldManager::GenerateEvents()
{
    auto Config = GetConfig();
    auto CategoryTable = Config ? Config->GetCategoryTable().LoadSynchronous() : nullptr;

    if (!CategoryTable || !GetCachedGameState())
        return;

    auto& EventInstances = GetEventInstances();
    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (int32 Index = EventInstances.Num() - 1; Index >= 0; Index--)
    {
        if (!EventInstances[Index].GetSpawnedActors().Num() && (GetPendingActorSpawnEventRequestsOffset() > 0 || !EventInstances[Index].GetPendingSpawnRequests().Num()) && TimeSeconds >= EventInstances[Index].GetRespawnTime())
            EventInstances.RemoveAt(Index);
    }

    int32 ActorBudget = (int32)Config->GetMaxActorCount().GetValueAtLevel(0.0f) - GetActorCount(nullptr, nullptr);
    TArray<FFortAthenaLivingWorldCategory*> Categories;

    for (auto& CategoryPair : CategoryTable->RowMap)
        Categories.Add((FFortAthenaLivingWorldCategory*)CategoryPair.Value);

    if (FFortAthenaLivingWorldCategory::StaticStruct()->FindPropertyByName(FName(TEXT("Priority"))))
        Sort(Categories.GetData(), Categories.Num(), [](FFortAthenaLivingWorldCategory& A, FFortAthenaLivingWorldCategory& B) { return A.GetPriority() > B.GetPriority(); });

    for (auto Category : Categories)
    {
        const bool bExcludeFromGlobalAIBudget = Category->GetbExcludeFromGlobalAIBudget();

        if (!bExcludeFromGlobalAIBudget && ActorBudget <= 0)
            continue;

        if (!Category->IsRequiredByPlaylist(GetPlaylistContextTags()))
            continue;

        auto EventTable = Category->GetEventTable().LoadSynchronous();

        if (!EventTable)
            continue;

        int32 CategoryBudget = Category->GetMaxCount() - GetActorCount(Category, nullptr);
        TArray<FFortAthenaLivingWorldEvent*> ActiveEvents;
        float TotalWeight = 0.0f;

        if (!bExcludeFromGlobalAIBudget)
            CategoryBudget = FMath::Min(ActorBudget, CategoryBudget);

        ComputeActiveEvents(EventTable, ActiveEvents, TotalWeight);

        for (int32 Attempt = CategoryBudget; Attempt > 0 && CategoryBudget > 0 && TotalWeight > 0.0f; Attempt--)
        {
            const float Pick = FMath::FRand() * TotalWeight;
            float Weight = 0.0f;
            FFortAthenaLivingWorldEvent* Event = nullptr;

            for (auto ActiveEvent : ActiveEvents)
            {
                if (ActiveEvent->GetWeight() <= 0.0f)
                    continue;

                Weight += ActiveEvent->GetWeight();

                if (Pick <= Weight)
                {
                    Event = ActiveEvent;
                    break;
                }
            }

            if (!Event)
                break;

            auto EventData = Event->GetEventData().LoadSynchronous();
            const int32 EventBudget = FMath::Min(CategoryBudget, GetEventBudget(Event));
            const int32 NumActorDescriptions = EventData ? EventData->GetActorDescriptions().Num() : 0;
            auto MinActorCountToSpawn = EventData ? EventData->GetMinActorCountToSpawn() : nullptr;
            auto MaxActorCountToSpawn = EventData ? EventData->GetMaxActorCountToSpawn() : nullptr;
            const int32 MinActorCount = MinActorCountToSpawn ? (int32)MinActorCountToSpawn->GetValueAtLevel(0.0f) : NumActorDescriptions;
            const int32 MaxActorCount = MaxActorCountToSpawn ? (int32)MaxActorCountToSpawn->GetValueAtLevel(0.0f) : NumActorDescriptions;

            if (!NumActorDescriptions || MinActorCount > EventBudget)
            {
                ActiveEvents.RemoveSingle(Event);
                TotalWeight -= Event->GetWeight();
                continue;
            }

            const int32 ActorCount = FMath::Min(FMath::Min(FMath::RandRange(MinActorCount, MaxActorCount), NumActorDescriptions), EventBudget);

            if (ActorCount <= 0)
                continue;

            const FFortAthenaLivingWorldPointProviderFilterRules PointFilter(Event, EventData, ActorCount, FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules() ? FindOrCreateEventRuntimeData(Event).DensityComputationRange : 0.0f, GetMaxActorDensity());
            auto PointProvider = GetRandomPointProvider(EventData->GetProviderFiltersTagQuery(), PointFilter);

            if (!PointProvider)
            {
                ActiveEvents.RemoveSingle(Event);
                TotalWeight -= Event->GetWeight();
                continue;
            }

            auto PointProviderInterface = (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass());

            for (int32 Index = 0; Index < ActorCount; Index++)
                PointProviderInterface->OnEventRequested();

            FLivingWorldEventRequest EventRequest;
            EventRequest.LivingWorldManager = this;
            EventRequest.Category = Category;
            EventRequest.Event = Event;
            EventRequest.EventData = EventData;
            EventRequest.PointProvider = PointProvider;
            EventRequest.ActorCountToSpawn = ActorCount;

            PendingEventRequests.Add(EventRequest);
            FindOrCreateEventRuntimeData(Event).SpawnedCount += ActorCount;

            CategoryBudget -= ActorCount;

            if (!bExcludeFromGlobalAIBudget)
                ActorBudget -= ActorCount;
        }
    }
}

void UFortAthenaLivingWorldManager::ProcessEventRequests()
{
    auto Config = GetConfig();
    int32 EventSpawnBudget = Config ? (int32)Config->GetMaxEventSpawnPerTick().GetValueAtLevel(0.0f) : 0;

    for (int32 Index = 0; Index < PendingEventRequests.Num() && EventSpawnBudget > 0;)
    {
        auto LivingWorldManager = PendingEventRequests[Index].LivingWorldManager.Get();

        if (LivingWorldManager && LivingWorldManager != this)
        {
            Index++;
            continue;
        }

        const FLivingWorldEventRequest EventRequest = PendingEventRequests[Index];
        PendingEventRequests.RemoveAt(Index);

        if (!LivingWorldManager)
            continue;

        RequestSpawnToAISpawnerSystem(EventRequest);
        EventSpawnBudget--;
    }
}

void UFortAthenaLivingWorldManager::ProcessEQSRequests()
{
    for (int32 Index = PendingEQSSpawns.Num() - 1; Index >= 0; Index--)
    {
        if (!PendingEQSSpawns[Index].LivingWorldManager.Get())
            PendingEQSSpawns.RemoveAt(Index);
    }

    for (int32 Index = 0; Index < PendingEQSSpawns.Num(); Index++)
    {
        auto& PendingEQSSpawn = PendingEQSSpawns[Index];

        if (PendingEQSSpawn.LivingWorldManager.Get() != this)
            continue;

        if (PendingEQSSpawn.QueryID != INDEX_NONE && GetWorld()->GetTimeSeconds() < PendingEQSSpawn.QueryStartTime + 5.0f)
            return;

        auto EQSHelper = GetEQSHelper();
        auto RootComponent = EQSHelper ? EQSHelper->GetRootComponent() : nullptr;
        auto SpawnAroundEnvironmentQuery = (UEnvQuery*)PendingEQSSpawn.SpawnAroundEnvironmentQuery.Get();

        if (PendingEQSSpawn.QueryID == INDEX_NONE && RootComponent && SpawnAroundEnvironmentQuery)
        {
            RootComponent->K2_SetWorldLocationAndRotation(PendingEQSSpawn.DefaultPosition, PendingEQSSpawn.DefaultRotation, false, true);

            FEnvQueryRequest QueryRequest(SpawnAroundEnvironmentQuery, EQSHelper);
            PendingEQSSpawn.QueryStartTime = GetWorld()->GetTimeSeconds();
            PendingEQSSpawn.QueryID = QueryRequest.Execute(EEnvQueryRunMode::AllMatching, FQueryFinishedSignature::CreateUObject(this, &UFortAthenaLivingWorldManager::OnEQSRequestFinished));

            if (PendingEQSSpawn.QueryID != INDEX_NONE)
                return;
        }

        const FLivingWorldPendingEQSSpawn FailedEQSSpawn = PendingEQSSpawn;
        PendingEQSSpawns.RemoveAt(Index);

        SpawnFromEQSRequest(FailedEQSSpawn, nullptr);
        return;
    }
}

void UFortAthenaLivingWorldManager::OnEQSRequestFinished(TSharedPtr<FEnvQueryResult> Result)
{
    for (int32 Index = 0; Index < PendingEQSSpawns.Num(); Index++)
    {
        if (!Result.IsValid() || PendingEQSSpawns[Index].LivingWorldManager.Get() != this || PendingEQSSpawns[Index].QueryID != Result->QueryID)
            continue;

        const FLivingWorldPendingEQSSpawn PendingEQSSpawn = PendingEQSSpawns[Index];
        PendingEQSSpawns.RemoveAt(Index);

        const int32 ItemCount = Result->Items.Num();
        const FVector Location = ItemCount > 0 ? Result->GetItemAsLocation(FMath::RandHelper(ItemCount)) : FVector(ForceInit);

        SpawnFromEQSRequest(PendingEQSSpawn, ItemCount > 0 ? &Location : nullptr);
        return;
    }
}

void UFortAthenaLivingWorldManager::SpawnFromEQSRequest(const FLivingWorldPendingEQSSpawn& PendingEQSSpawn, const FVector* Location)
{
    auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;
    auto SpawnerDataClass = PendingEQSSpawn.SpawnerDataClass.Get();
    auto ActorClass = PendingEQSSpawn.ActorClass.Get();
    FVector* PendingLocation = nullptr;
    int32* PendingRequestID = nullptr;

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        for (auto& PendingActorSpawnEventRequest : GetPendingActorSpawnEventRequests())
        {
            for (auto& RequestInfo : PendingActorSpawnEventRequest.RequestInfos)
            {
                if (RequestInfo.RequestID == PendingEQSSpawn.RequestID)
                {
                    PendingLocation = &RequestInfo.Location;
                    PendingRequestID = &RequestInfo.RequestID;
                }
            }
        }
    }
    else
    {
        for (auto& EventInstance : GetEventInstances())
        {
            for (auto& PendingSpawnRequest : EventInstance.GetPendingSpawnRequests())
            {
                if (PendingSpawnRequest.RequestID == PendingEQSSpawn.RequestID)
                {
                    PendingLocation = &PendingSpawnRequest.Location;
                    PendingRequestID = &PendingSpawnRequest.RequestID;
                }
            }
        }
    }

    if (Location && PendingLocation)
        *PendingLocation = *Location;

    if (Location && SpawnerDataClass && AISpawner)
    {
        const FTransform SpawnTransform(PendingEQSSpawn.DefaultRotation, *Location, FVector(1.0f, 1.0f, 1.0f));
        auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this);
        const int32 RequestID = ComponentList ? AISpawner->RequestSpawn(ComponentList, SpawnTransform, TDelegate<void(AActor*, int32)>::CreateUObject(this, &UFortAthenaLivingWorldManager::OnActorSpawned)) : INDEX_NONE;

        if (RequestID != INDEX_NONE && PendingRequestID)
        {
            *PendingRequestID = RequestID;
            return;
        }
    }

    if (Location && !SpawnerDataClass && ActorClass)
    {
        const FTransform SpawnTransform(PendingEQSSpawn.DefaultRotation, *Location, FVector(1.0f, 1.0f, 1.0f));

        OnActorSpawned(GetWorld()->SpawnActor(ActorClass, &SpawnTransform), PendingEQSSpawn.RequestID);
        return;
    }

    OnActorSpawned(nullptr, PendingEQSSpawn.RequestID);
}

void UFortAthenaLivingWorldManager::OnActorSpawned(AActor* SpawnedActor, int32 RequestID)
{
    static auto NativeOnActorSpawned = Anchor::Optional<void (*)(UFortAthenaLivingWorldManager*, AActor*, int32)>(TEXT("UFortAthenaLivingWorldManager::OnActorSpawned"))
        .If(GetPendingActorSpawnEventRequestsOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnActorSpawned"))
        .Get();

    if (NativeOnActorSpawned)
        return NativeOnActorSpawned(this, SpawnedActor, RequestID);

    for (auto& EventInstance : GetEventInstances())
    {
        auto& PendingSpawnRequests = EventInstance.GetPendingSpawnRequests();

        for (int32 Index = 0; Index < PendingSpawnRequests.Num(); Index++)
        {
            if (PendingSpawnRequests[Index].RequestID != RequestID)
                continue;

            auto PointProvider = PendingSpawnRequests[Index].PointProvider.Get();
            const FVector PositionFromProvider = PendingSpawnRequests[Index].Location;
            PendingSpawnRequests.RemoveAt(Index);

            if (SpawnedActor)
            {
                OnActorSpawned(EventInstance, SpawnedActor, PointProvider, PositionFromProvider);
            }
            else if (auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr)
            {
                PointProviderInterface->OnSpawnedActorFailed();
            }

            return;
        }
    }
}

void UFortAthenaLivingWorldManager::OnActorSpawned(FLivingWorldEventInstance& EventInstance, AActor* SpawnedActor, AActor* PointProvider, const FVector& PositionFromProvider)
{
    auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;

    EventInstance.GetSpawnedActors().Add(SpawnedActor);

    if (auto SpawnedActorPointProviders = EventInstance.GetSpawnedActorPointProviders())
    {
        FScriptInterface SpawnedActorPointProvider;
        SpawnedActorPointProvider.SetObject(PointProviderInterface ? PointProvider : nullptr);
        SpawnedActorPointProvider.SetInterface(PointProviderInterface);

        SpawnedActorPointProviders->Add(SpawnedActorPointProvider);
    }

    FScriptDelegate OnSpawnedActorDestroyDelegate;
    OnSpawnedActorDestroyDelegate.BindUFunction(this, FName(TEXT("OnSpawnedActorDestroy")));
    SpawnedActor->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::Add(SpawnedActor, FName(TEXT("OnDestroyed")), OnSpawnedActorDestroyDelegate);

    if (auto SpawnedPawns = GetSpawnedPawns())
        SpawnedPawns->Add(SpawnedActor);

    if (auto ActorDensityGrid = GetActorDensityGrid())
        ActorDensityGrid->AddActor(SpawnedActor);

    if (PointProviderInterface)
        PointProviderInterface->OnSpawnedActor(SpawnedActor, PositionFromProvider);
}

void UFortAthenaLivingWorldManager::ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight)
{
    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (auto& EventPair : EventTable->RowMap)
    {
        auto Event = (FFortAthenaLivingWorldEvent*)EventPair.Value;
        auto EventData = Event->GetEventData().LoadSynchronous();

        if (!EventData || !IsEventActive(Event, EventData))
            continue;

        auto& RuntimeData = FindOrCreateEventRuntimeData(Event);

        if (RuntimeData.ActivationTime == -1.0f)
            RuntimeData.ActivationTime = FMath::FRand() * (Event->GetActivationMaxDelay() - Event->GetActivationMinDelay()) + Event->GetActivationMinDelay() + TimeSeconds;

        if (TimeSeconds < RuntimeData.ActivationTime)
            continue;

        ActiveEvents.Add(Event);
        TotalWeight += Event->GetWeight();
    }
}

bool UFortAthenaLivingWorldManager::IsEventActive(FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData)
{
    static auto NativeIsEventActive = Anchor::Optional<bool (*)(UFortAthenaLivingWorldManager*, FFortAthenaLivingWorldEvent*, UFortAthenaLivingWorldEventData*, bool, EAthenaGamePhaseStep, int32)>(TEXT("UFortAthenaLivingWorldManager::IsEventActive"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("mov [rsp+0x20], ?l")
        .Find("call 0x*")
        .Target()
        .Get();

    auto GameState = GetCachedGameState();

    if (NativeIsEventActive)
        return NativeIsEventActive(this, Event, EventData, false, GameState->GetGamePhaseStep(), GameState->GetSafeZonePhase());

    return Event->IsActive(GetPlaylistContextTags()) && GameState->GetGamePhaseStep() >= Event->GetActivationGamePhaseStep() && GameState->GetSafeZonePhase() >= Event->GetActivationSafeZoneIndex();
}

int32 UFortAthenaLivingWorldManager::GetEventBudget(FFortAthenaLivingWorldEvent* Event)
{
    const int32 SafeZonePhase = GetCachedGameState()->GetSafeZonePhase();
    auto& MaxCount = Event->GetMaxCount();
    const float MaxCountValue = MaxCount.Num() ? MaxCount[SafeZonePhase < MaxCount.Num() ? SafeZonePhase : MaxCount.Num() - 1] : 0.0f;
    const int32 Budget = FMath::Max(0, (int32)MaxCountValue - GetActorCount(nullptr, Event));
    auto& MaxSpawnedCount = Event->GetMaxSpawnedCount();

    if (MaxSpawnedCount.Num() && Event->GetMaxSpawnedCount(SafeZonePhase) == -1)
        return Budget;

    return FMath::Min(Budget, FMath::Max(0, Event->GetMaxSpawnedCount(SafeZonePhase) - FindOrCreateEventRuntimeData(Event).SpawnedCount));
}

int32 UFortAthenaLivingWorldManager::GetActorCount(const FFortAthenaLivingWorldCategory* Category, const FFortAthenaLivingWorldEvent* Event)
{
    int32 ActorCount = 0;

    for (auto& EventInstance : GetEventInstances())
    {
        if (!Category && !Event && EventInstance.Category && EventInstance.Category->GetbExcludeFromGlobalAIBudget())
            continue;

        if ((!Category || EventInstance.Category == Category) && (!Event || EventInstance.Event == Event))
            ActorCount += EventInstance.GetActorCount();
    }

    for (auto& EventRequest : PendingEventRequests)
    {
        auto EventData = EventRequest.EventData.Get();

        if (!Category && !Event && EventRequest.Category && EventRequest.Category->GetbExcludeFromGlobalAIBudget())
            continue;

        if (EventData && EventRequest.LivingWorldManager.Get() == this && (!Category || EventRequest.Category == Category) && (!Event || EventRequest.Event == Event))
            ActorCount += EventRequest.ActorCountToSpawn;
    }

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        for (auto& PendingActorSpawnEventRequest : GetPendingActorSpawnEventRequests())
        {
            auto& RequestData = PendingActorSpawnEventRequest.RequestData;

            if (!Category && !Event && RequestData.Category && RequestData.Category->GetbExcludeFromGlobalAIBudget())
                continue;

            if ((!Category || RequestData.Category == Category) && (!Event || RequestData.Event == Event))
                ActorCount += RequestData.ActorCountToSpawn;
        }
    }

    return ActorCount;
}

AActor* UFortAthenaLivingWorldManager::GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter)
{
    TArray<AActor*> PointProviders;
    TArray<AActor*> ValidPointProviders;

    if (HasPointProviderInterfaces())
    {
        for (auto& PointProvider : GetPointProviderInterfaces())
            PointProviders.Add((AActor*)PointProvider.GetObject());
    }
    else
    {
        PointProviders = GetPointProviders();
    }

    for (auto RuntimePointProvider : GetRuntimePointProviderList())
        PointProviders.AddUnique(RuntimePointProvider);

    for (auto PointProvider : PointProviders)
    {
        auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;

        if (!PointProviderInterface)
            continue;

        FGameplayTagContainer FiltersTags;
        PointProviderInterface->GetFiltersTags(FiltersTags);

        if (!ProviderFiltersTagQuery.IsEmpty() && !ProviderFiltersTagQuery.Matches(FiltersTags))
            continue;

        if (PointProviderInterface->IsEnabled(PointFilter))
            ValidPointProviders.Add(PointProvider);
    }

    return ValidPointProviders.Num() ? ValidPointProviders[FMath::RandHelper(ValidPointProviders.Num())] : nullptr;
}

bool UFortAthenaLivingWorldManager::RequestSpawnToAISpawnerSystem(const FLivingWorldEventRequest& EventRequest)
{
    auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;
    auto Event = EventRequest.Event;
    auto EventData = EventRequest.EventData.Get();
    auto PointProvider = EventRequest.PointProvider.Get();
    auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;
    const int32 ActorCount = EventData ? FMath::Min(EventRequest.ActorCountToSpawn, EventData->GetActorDescriptions().Num()) : 0;
    const FFortAthenaLivingWorldPointProviderFilterRules PointFilter(Event, EventData, ActorCount, FFortAthenaLivingWorldPointProviderFilterRules::HasDensityRules() ? FindOrCreateEventRuntimeData(Event).DensityComputationRange : 0.0f, GetMaxActorDensity());
    const auto SpawnedCallback = TDelegate<void(AActor*, int32)>::CreateUObject(this, &UFortAthenaLivingWorldManager::OnActorSpawned);
    auto Config = GetConfig();
    auto DensityComputationDistance = Config ? Config->GetDensityComputationDistance() : nullptr;
    const float ActorDensityComputationRadius = DensityComputationDistance ? DensityComputationDistance->GetValueAtLevel(0.0f) : 0.0f;
    TArray<FLivingWorldPendingSpawnRequest> PendingSpawnRequests;
    TArray<AActor*> SpawnedActors;
    TArray<FVector> SpawnedActorPositions;
    bool bHasDefaultPosition = false;
    FVector DefaultPosition(ForceInit);
    FRotator DefaultRotation{ 0.0f, 0.0f, 0.0f };

    for (int32 Index = 0; PointProviderInterface && Index < ActorCount; Index++)
    {
        auto& ActorDescription = EventData->GetActorDescriptions()[Index];
        auto SpawnerDataClass = GetSpawnerDataClass(ActorDescription);
        auto ActorClass = SpawnerDataClass ? nullptr : ActorDescription.GetActorClass().LoadSynchronous();
        FVector Location(ForceInit);
        FRotator Rotation{ 0.0f, 0.0f, 0.0f };

        if (SpawnerDataClass ? !AISpawner : !ActorClass || !ActorClass->IsChildOf(AActor::StaticClass()))
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        if (ActorDescription.GetbSpawnAroundDefaultPoint() && bHasDefaultPosition)
        {
            Location = DefaultPosition;
            Rotation = DefaultRotation;
        }
        else if (PointProviderInterface->GetValidLocation(PointFilter, ActorDensityComputationRadius, Location, Rotation))
        {
            if (!bHasDefaultPosition)
            {
                DefaultPosition = Location;
                DefaultRotation = Rotation;
                bHasDefaultPosition = true;
            }
        }
        else
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        auto SpawnAroundEnvironmentQuery = ActorDescription.GetbSpawnAroundDefaultPoint() ? ActorDescription.GetSpawnAroundEnvironmentQuery() : nullptr;

        if (SpawnAroundEnvironmentQuery && GetEQSHelper())
        {
            FLivingWorldPendingEQSSpawn PendingEQSSpawn;
            PendingEQSSpawn.LivingWorldManager = this;
            PendingEQSSpawn.PointProvider = PointProvider;
            PendingEQSSpawn.SpawnerDataClass = SpawnerDataClass;
            PendingEQSSpawn.ActorClass = ActorClass;
            PendingEQSSpawn.SpawnAroundEnvironmentQuery = (UObject*)SpawnAroundEnvironmentQuery;
            PendingEQSSpawn.DefaultPosition = DefaultPosition;
            PendingEQSSpawn.DefaultRotation = DefaultRotation;
            PendingEQSSpawn.RequestID = --NextEQSSpawnRequestID;

            PendingEQSSpawns.Add(PendingEQSSpawn);

            FLivingWorldPendingSpawnRequest PendingSpawnRequest;
            PendingSpawnRequest.RequestID = PendingEQSSpawn.RequestID;
            PendingSpawnRequest.PointProvider = PointProvider;
            PendingSpawnRequest.Location = DefaultPosition;

            PendingSpawnRequests.Add(PendingSpawnRequest);
            continue;
        }

        const FTransform SpawnTransform(Rotation, Location, FVector(1.0f, 1.0f, 1.0f));

        if (!SpawnerDataClass)
        {
            auto SpawnedActor = GetWorld()->SpawnActor(ActorClass, &SpawnTransform);

            if (!SpawnedActor)
            {
                PointProviderInterface->OnSpawnedActorFailed();
                continue;
            }

            SpawnedActors.Add(SpawnedActor);
            SpawnedActorPositions.Add(Location);
            continue;
        }

        auto ComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this);
        const int32 RequestID = ComponentList ? AISpawner->RequestSpawn(ComponentList, SpawnTransform, SpawnedCallback) : INDEX_NONE;

        if (RequestID == INDEX_NONE)
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        FLivingWorldPendingSpawnRequest PendingSpawnRequest;
        PendingSpawnRequest.RequestID = RequestID;
        PendingSpawnRequest.PointProvider = PointProvider;
        PendingSpawnRequest.Location = Location;

        PendingSpawnRequests.Add(PendingSpawnRequest);
    }

    const int32 SpawnedCount = PendingSpawnRequests.Num() + SpawnedActors.Num();

    FindOrCreateEventRuntimeData(Event).SpawnedCount -= ActorCount - SpawnedCount;

    if (!SpawnedCount)
        return false;

    if (GetPendingActorSpawnEventRequestsOffset() > 0)
    {
        auto& PendingActorSpawnEventRequest = *new(GetPendingActorSpawnEventRequests()) FLivingWorldPendingActorSpawnEventRequest();
        TArray<int32> SpawnedActorRequestIDs;

        PendingActorSpawnEventRequest.RequestData.Category = EventRequest.Category;
        PendingActorSpawnEventRequest.RequestData.Event = Event;
        PendingActorSpawnEventRequest.RequestData.EventData = EventData;
        PendingActorSpawnEventRequest.RequestData.ActorCountToSpawn = SpawnedCount;

        for (int32 Index = 0; Index < SpawnedCount; Index++)
        {
            auto& RequestInfo = *new(PendingActorSpawnEventRequest.RequestInfos) FLivingWorldPendingActorSpawnEventRequest::FRequestInfo();

            RequestInfo.Location = Index < PendingSpawnRequests.Num() ? PendingSpawnRequests[Index].Location : SpawnedActorPositions[Index - PendingSpawnRequests.Num()];
            RequestInfo.RequestID = Index < PendingSpawnRequests.Num() ? PendingSpawnRequests[Index].RequestID : SpawnedActorRequestIDs[SpawnedActorRequestIDs.Add(--NextEQSSpawnRequestID)];
            RequestInfo.PreRegistrationHandle = INDEX_NONE;
            RequestInfo.PointProvider.SetObject(PointProvider);
            RequestInfo.PointProvider.SetInterface(PointProviderInterface);
        }

        for (int32 Index = 0; Index < SpawnedActors.Num(); Index++)
            OnActorSpawned(SpawnedActors[Index], SpawnedActorRequestIDs[Index]);

        return true;
    }

    auto& EventInstances = GetEventInstances();
    auto& EventInstance = EventInstances[EventInstances.AddUninitialized()];

    memset(&EventInstance, 0, FLivingWorldEventInstance::GetTypeSize());

    EventInstance.Category = EventRequest.Category;
    EventInstance.Event = Event;
    EventInstance.EventData = EventData;
    EventInstance.GetPendingSpawnRequests() = PendingSpawnRequests;
    EventInstance.GetActorCount() = SpawnedCount;

    for (int32 Index = 0; Index < SpawnedActors.Num(); Index++)
        OnActorSpawned(EventInstance, SpawnedActors[Index], PointProvider, SpawnedActorPositions[Index]);

    return true;
}

UClass* UFortAthenaLivingWorldManager::GetSpawnerDataClass(FFortAthenaLivingWorldEventDataActorSpawnDescription& ActorDescription)
{
    if (auto SpawnerData = ActorDescription.GetSpawnerData().LoadSynchronous())
        return SpawnerData;

    TArray<UClass*> SpawnerDataClasses;

    for (auto LoadedSpawnerDataClass : GetLoadedSpawnerDataClass())
    {
        auto SpawnerData = LoadedSpawnerDataClass ? Cast<UFortAthenaAISpawnerData>(LoadedSpawnerDataClass->GetDefaultObject()) : nullptr;

        if (SpawnerData && ActorDescription.GetSpawnerDataTagQuery().Matches(SpawnerData->GetDescriptorTag()))
            SpawnerDataClasses.Add(LoadedSpawnerDataClass);
    }

    return SpawnerDataClasses.Num() ? SpawnerDataClasses[FMath::RandHelper(SpawnerDataClasses.Num())] : nullptr;
}

UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData& UFortAthenaLivingWorldManager::FindOrCreateEventRuntimeData(const FFortAthenaLivingWorldEvent* Event)
{
    static auto NativeFindOrCreateEventRuntimeData = Anchor::Optional<FLivingWorldEventRuntimeData* (*)(UFortAthenaLivingWorldManager*, const FFortAthenaLivingWorldEvent*)>(TEXT("UFortAthenaLivingWorldManager::FindOrCreateEventRuntimeData"))
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("mov rcx, *; call 0x*; mov rbx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, *; mov rcx, *; mov *, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rdx, *; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto NativeFindOrAddEventRuntimeData = Anchor::Optional<FLivingWorldEventRuntimeData* (*)(const FFortAthenaLivingWorldEvent*, void*)>(TEXT("TMap<const FFortAthenaLivingWorldEvent*, FLivingWorldEventRuntimeData>::FindOrAdd"))
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .If(!NativeFindOrCreateEventRuntimeData && StaticClass() && StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget")))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto EventRuntimeDataMap = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventRuntimeDataMap"))
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Optional()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Or()
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]")
        .Find("call 0x*", 3)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Or()
        .If(NativeFindOrAddEventRuntimeData != nullptr)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("mov rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, [*+0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rdx, [*+0x*]; mov [rbp+0x*], rax; mov rcx, *; call 0x*")
        .Disp();

    if (NativeFindOrCreateEventRuntimeData)
        return *NativeFindOrCreateEventRuntimeData(this, Event);

    if (NativeFindOrAddEventRuntimeData && EventRuntimeDataMap > 0)
        return *NativeFindOrAddEventRuntimeData(Event, (uint8*)this + EventRuntimeDataMap);

    for (int32 Index = EventRuntimeData.Num() - 1; Index >= 0; Index--)
    {
        if (EventRuntimeData[Index].LivingWorldManager.Get() != this)
            EventRuntimeData.RemoveAt(Index);
        else if (EventRuntimeData[Index].Event == Event)
            return EventRuntimeData[Index].RuntimeData;
    }

    FLivingWorldEventRuntimeDataEntry RuntimeDataEntry;
    RuntimeDataEntry.LivingWorldManager = this;
    RuntimeDataEntry.Event = Event;

    return EventRuntimeData[EventRuntimeData.Add(RuntimeDataEntry)].RuntimeData;
}

UFortAthenaLivingWorldConfigData* UFortAthenaLivingWorldManager::GetConfig()
{
    static auto CachedConfig = GetClass()->FindPropertyByName(FName(TEXT("CachedConfig")));

    return CachedConfig ? *CachedConfig->ContainerPtrToValuePtr<UFortAthenaLivingWorldConfigData*>(this) : GetLagerConfig().LoadSynchronous();
}

void UFortAthenaLivingWorldManager::AddPointProvider(AActor* PointProvider)
{
    static auto AddPointProviderToEventRuntimeData = Anchor::Optional<void (*)(UFortAthenaLivingWorldManager*, const FScriptInterface*)>(TEXT("UFortAthenaLivingWorldManager::AddPointProviderToEventRuntimeData"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:SetFiltersTags"))
        .Find("lea rdx, [rbp-0x*]; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!HasPointProviderInterfaces())
    {
        GetPointProviders().AddUnique(PointProvider);
        return;
    }

    auto& PointProviders = GetPointProviderInterfaces();

    for (auto& RegisteredPointProvider : PointProviders)
    {
        if (RegisteredPointProvider.GetObject() == PointProvider)
            return;
    }

    FScriptInterface NewPointProvider;
    NewPointProvider.SetObject(PointProvider);
    NewPointProvider.SetInterface(PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()));

    PointProviders.Add(NewPointProvider);

    if (AddPointProviderToEventRuntimeData)
        AddPointProviderToEventRuntimeData(this, &NewPointProvider);
}

void UFortAthenaLivingWorldManager::RemovePointProvider(AActor* PointProvider)
{
    if (!HasPointProviderInterfaces())
    {
        GetPointProviders().RemoveSingle(PointProvider);
        return;
    }

    auto& PointProviders = GetPointProviderInterfaces();

    for (int32 Index = 0; Index < PointProviders.Num(); Index++)
    {
        if (PointProviders[Index].GetObject() == PointProvider)
        {
            PointProviders.RemoveAt(Index);
            return;
        }
    }
}

TSoftObjectPtr<UFortAthenaLivingWorldConfigData>& UFortAthenaLivingWorldManager::GetLagerConfig()
{
    static auto LagerConfig = GetClass()->FindPropertyByName(FName(TEXT("LagerConfig")));

    return *LagerConfig->ContainerPtrToValuePtr<TSoftObjectPtr<UFortAthenaLivingWorldConfigData>>(this);
}

FScalableFloat& UFortAthenaLivingWorldManager::GetLagerEnabled()
{
    static auto LagerEnabled = GetClass()->FindPropertyByName(FName(TEXT("LagerEnabled")));

    return *LagerEnabled->ContainerPtrToValuePtr<FScalableFloat>(this);
}

bool UFortAthenaLivingWorldManager::HasPointProviderInterfaces()
{
    static auto PointProviders = (UArrayProperty*)GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return PointProviders->GetInner()->GetSize() == sizeof(FScriptInterface);
}

TArray<AActor*>& UFortAthenaLivingWorldManager::GetPointProviders()
{
    static auto PointProviders = GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return *PointProviders->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

TArray<FScriptInterface>& UFortAthenaLivingWorldManager::GetPointProviderInterfaces()
{
    static auto PointProviders = GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return *PointProviders->ContainerPtrToValuePtr<TArray<FScriptInterface>>(this);
}

TArray<UClass*>& UFortAthenaLivingWorldManager::GetLoadedSpawnerDataClass()
{
    static auto LoadedSpawnerDataClass = GetClass()->FindPropertyByName(FName(TEXT("LoadedSpawnerDataClass")));

    return *LoadedSpawnerDataClass->ContainerPtrToValuePtr<TArray<UClass*>>(this);
}

AFortGameStateAthena*& UFortAthenaLivingWorldManager::GetCachedGameState()
{
    static auto CachedGameState = GetClass()->FindPropertyByName(FName(TEXT("CachedGameState")));

    return *CachedGameState->ContainerPtrToValuePtr<AFortGameStateAthena*>(this);
}

TArray<AActor*>& UFortAthenaLivingWorldManager::GetRuntimePointProviderList()
{
    static auto RuntimePointProviderList = GetClass()->FindPropertyByName(FName(TEXT("RuntimePointProviderList")));

    return *RuntimePointProviderList->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

TRuntimeSizedArray<UFortAthenaLivingWorldManager::FLivingWorldEventInstance>& UFortAthenaLivingWorldManager::GetEventInstances()
{
    return *(TRuntimeSizedArray<FLivingWorldEventInstance>*)((uint8*)this + GetEventInstancesOffset());
}

TArray<UFortAthenaLivingWorldManager::FLivingWorldPendingActorSpawnEventRequest>& UFortAthenaLivingWorldManager::GetPendingActorSpawnEventRequests()
{
    return *(TArray<FLivingWorldPendingActorSpawnEventRequest>*)((uint8*)this + GetPendingActorSpawnEventRequestsOffset());
}

float& UFortAthenaLivingWorldManager::GetNextEventGenerationTime()
{
    static auto NextEventGenerationTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::NextEventGenerationTime"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("mov rcx, *; call 0x*; mov rbx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Find("movss [*+0x*], xmm0; call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*; mov rcx, *; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("movss [*+0x*], xmm0; call [rax+0x*]")
        .Disp();

    return *(float*)((uint8*)this + NextEventGenerationTime);
}

FGameplayTagContainer& UFortAthenaLivingWorldManager::GetPlaylistContextTags()
{
    static auto PlaylistContextTags = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::PlaylistContextTags"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
        .Or()
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("lea rcx, [*+0x*]; mov rdx, *; call 0x*")
        .Disp();

    return *(FGameplayTagContainer*)((uint8*)this + PlaylistContextTags);
}

UWorld** UFortAthenaLivingWorldManager::GetCachedWorld()
{
    static auto CachedWorld = GetClass()->FindPropertyByName(FName(TEXT("CachedWorld")));

    return CachedWorld ? CachedWorld->ContainerPtrToValuePtr<UWorld*>(this) : nullptr;
}

AActor* UFortAthenaLivingWorldManager::GetEQSHelper()
{
    static auto EQSHelper = GetClass()->FindPropertyByName(FName(TEXT("EQSHelper")));

    return EQSHelper ? *EQSHelper->ContainerPtrToValuePtr<AActor*>(this) : nullptr;
}

FFortAthenaLivingWorldDensityGrid* UFortAthenaLivingWorldManager::GetActorDensityGrid()
{
    static auto ActorDensityGrid = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::ActorDensityGrid"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Or()
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, r12; call 0x*")
        .Optional()
        .Disp();

    return ActorDensityGrid > 0 ? (FFortAthenaLivingWorldDensityGrid*)((uint8*)this + ActorDensityGrid) : nullptr;
}

TArray<AActor*>* UFortAthenaLivingWorldManager::GetSpawnedPawns()
{
    static auto SpawnedPawns = GetClass()->FindPropertyByName(FName(TEXT("SpawnedPawns")));

    return SpawnedPawns ? SpawnedPawns->ContainerPtrToValuePtr<TArray<AActor*>>(this) : nullptr;
}

float UFortAthenaLivingWorldManager::GetActorDensity(const FVector& Location, float Radius)
{
    static auto GetActorDensity = Anchor::Optional<float (*)(UFortAthenaLivingWorldManager*, const FVector&, float)>(TEXT("UFortAthenaLivingWorldManager::GetActorDensity"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("xorps xmm2, xmm2; movsd xmm0, qword ptr [rax]; lea rdx, [rsp+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movaps xmm2, xmm0; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto GetGridActorDensity = Anchor::Optional<int32 (*)(FFortAthenaLivingWorldDensityGrid*, const FVector&, float)>(TEXT("FFortAthenaLivingWorldDensityGrid::GetActorDensity"))
        .If(!GetActorDensity)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("lea rcx, [rdi+0x*]; movaps xmm2, xmm0; lea rdx, [rsp+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (GetActorDensity)
        return GetActorDensity(this, Location, Radius);

    auto ActorDensityGrid = GetActorDensityGrid();

    return GetGridActorDensity && ActorDensityGrid ? (float)GetGridActorDensity(ActorDensityGrid, Location, Radius) : 0.0f;
}

float UFortAthenaLivingWorldManager::GetMaxActorDensity()
{
    auto Config = GetConfig();
    auto MaxActorDensity = Config ? Config->GetMaxActorDensity() : nullptr;

    return MaxActorDensity ? MaxActorDensity->GetValueAtLevel(0.0f) : 0.0f;
}

void UFortAthenaLivingWorldManager::ClusterPoints(const TArray<FVector>& Points, TArray<FVector>& ClusteredPoints)
{
    auto Config = GetConfig();

    if (!Config)
        return;

    ClusteredPoints.Empty();

    if (Points.Num() <= 0)
        return;

    auto PointClusterSize = Config->GetPointClusterSize();
    const float HalfClusterSize = (PointClusterSize ? PointClusterSize->GetValueAtLevel(0.0f) : 0.0f) * 0.5f;

    if (HalfClusterSize <= 0.0f)
    {
        ClusteredPoints.Append(Points);
        return;
    }

    for (auto& Point : Points)
    {
        bool bInCluster = false;

        for (auto& ClusteredPoint : ClusteredPoints)
        {
            if (Point.X > ClusteredPoint.X - HalfClusterSize && Point.X < ClusteredPoint.X + HalfClusterSize && Point.Y > ClusteredPoint.Y - HalfClusterSize && Point.Y < ClusteredPoint.Y + HalfClusterSize)
            {
                bInCluster = true;
                break;
            }
        }

        if (!bInCluster)
            ClusteredPoints.Add(Point);
    }
}

int32 UFortAthenaLivingWorldManager::GetActorDensityBudget(const FVector& Location, float Radius)
{
    auto Config = GetConfig();
    auto MaxActorDensity = Config ? Config->GetMaxActorDensity() : nullptr;

    return MaxActorDensity ? FMath::Max(0, (int32)(MaxActorDensity->GetValueAtLevel(0.0f) - GetActorDensity(Location, Radius))) : 0;
}
