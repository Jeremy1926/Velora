#include "pch.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldCategory.h"
#include "FortAthenaLivingWorldConfigData.h"
#include "FortAthenaLivingWorldEvent.h"
#include "FortAthenaLivingWorldEventData.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "Athena/AI/AthenaAISpawner.h"
#include "Athena/AI/AthenaAISystem.h"
#include "Athena/AI/FortAthenaAISpawnerData.h"
#include "Athena/FortGameStateAthena.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/SparseDelegate.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventRuntimeData> UFortAthenaLivingWorldManager::EventRuntimeData;

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
        .Disp();

    return EventInstances;
}

void UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(AActor* PointProvider)
{
    auto World = PointProvider ? PointProvider->GetWorld() : nullptr;
    auto GameState = World ? World->GetGameState() : nullptr;
    auto LivingWorldManager = GameState ? (UFortAthenaLivingWorldManager*)GameState->FindComponentByClass(StaticClass()) : nullptr;

    if (!LivingWorldManager || !LivingWorldManager->IsLivingWorldEnabled() || !GameState->HasAuthority())
        return;

    LivingWorldManager->GetPointProviders().AddUnique(PointProvider);
}

bool UFortAthenaLivingWorldManager::IsLivingWorldEnabled()
{
    return GetLagerEnabled().GetValueAtLevel(0.0f) > 0.0f && !FParse::Param(FCommandLine::Get(), TEXT("NoLager"));
}

void UFortAthenaLivingWorldManager::TickComponent(float DeltaTime)
{
    auto World = GetWorld();
    auto& NextEventGenerationTime = GetNextEventGenerationTime();

    if (!World || NextEventGenerationTime < 0.0f || World->GetTimeSeconds() < NextEventGenerationTime)
        return;

    auto LagerConfig = GetLagerConfig().LoadSynchronous();

    if (!LagerConfig)
        return;

    NextEventGenerationTime = World->GetTimeSeconds() + LagerConfig->GetMinimumEventGenerationInterval().GetValueAtLevel(0.0f);

    GenerateEvents();
}

void UFortAthenaLivingWorldManager::GenerateEvents()
{
    auto LagerConfig = GetLagerConfig().LoadSynchronous();
    auto CategoryTable = LagerConfig ? LagerConfig->GetCategoryTable().LoadSynchronous() : nullptr;

    if (!CategoryTable || !GetCachedGameState())
        return;

    auto& EventInstances = GetEventInstances();
    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (int32 Index = EventInstances.Num() - 1; Index >= 0; Index--)
    {
        if (!EventInstances[Index].SpawnedActors.Num() && !EventInstances[Index].PendingRequestIDs.Num() && TimeSeconds >= EventInstances[Index].RespawnTime)
            EventInstances.RemoveAt(Index);
    }

    int32 ActorBudget = (int32)LagerConfig->GetMaxActorCount().GetValueAtLevel(0.0f) - GetActorCount(nullptr, nullptr);
    int32 EventSpawnBudget = (int32)LagerConfig->GetMaxEventSpawnPerTick().GetValueAtLevel(0.0f);

    for (auto& CategoryPair : CategoryTable->RowMap)
    {
        if (ActorBudget <= 0 || EventSpawnBudget <= 0)
            break;

        auto Category = (FFortAthenaLivingWorldCategory*)CategoryPair.Value;

        if (!Category->GetRequirePlaylistTags().IsEmpty() && !GetPlaylistContextTags().HasAny(Category->GetRequirePlaylistTags()))
            continue;

        auto EventTable = Category->GetEventTable().LoadSynchronous();

        if (!EventTable)
            continue;

        int32 CategoryBudget = FMath::Min(ActorBudget, Category->GetMaxCount() - GetActorCount(Category, nullptr));
        TArray<FFortAthenaLivingWorldEvent*> ActiveEvents;
        float TotalWeight = 0.0f;

        ComputeActiveEvents(EventTable, ActiveEvents, TotalWeight);

        while (CategoryBudget > 0 && EventSpawnBudget > 0 && TotalWeight > 0.0f)
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

            ActiveEvents.RemoveSingle(Event);
            TotalWeight -= Event->GetWeight();

            auto EventData = Event->GetEventData().LoadSynchronous();
            const int32 ActorCount = EventData ? EventData->GetActorDescriptions().Num() : 0;

            if (!ActorCount || ActorCount > FMath::Min(CategoryBudget, GetEventBudget(Event)) || !RequestSpawnToAISpawnerSystem(Category, Event, EventData))
                continue;

            CategoryBudget -= ActorCount;
            ActorBudget -= ActorCount;
            EventSpawnBudget--;
        }
    }
}

void UFortAthenaLivingWorldManager::OnActorSpawned(AActor* SpawnedActor, int32 RequestID)
{
    for (auto& EventInstance : GetEventInstances())
    {
        if (!EventInstance.PendingRequestIDs.RemoveSingle(RequestID))
            continue;

        auto PointProvider = EventInstance.PointProvider.Get();
        auto PointProviderInterface = PointProvider ? (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) : nullptr;

        if (!SpawnedActor)
        {
            if (PointProviderInterface)
                PointProviderInterface->OnSpawnedActorFailed();

            return;
        }

        EventInstance.SpawnedActors.Add(SpawnedActor);

        FScriptDelegate OnSpawnedActorDestroyDelegate;
        OnSpawnedActorDestroyDelegate.BindUFunction(this, FName(TEXT("OnSpawnedActorDestroy")));
        SpawnedActor->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::Add(SpawnedActor, FName(TEXT("OnDestroyed")), OnSpawnedActorDestroyDelegate);

        if (PointProviderInterface)
            PointProviderInterface->OnSpawnedActor(SpawnedActor);

        return;
    }
}

void UFortAthenaLivingWorldManager::ComputeActiveEvents(UDataTable* EventTable, TArray<FFortAthenaLivingWorldEvent*>& ActiveEvents, float& TotalWeight)
{
    auto GameState = GetCachedGameState();
    const float TimeSeconds = GetWorld()->GetTimeSeconds();

    for (auto& EventPair : EventTable->RowMap)
    {
        auto Event = (FFortAthenaLivingWorldEvent*)EventPair.Value;

        if (!Event->IsActive(GetPlaylistContextTags()) || !Event->GetEventData().LoadSynchronous())
            continue;

        if (GameState->GetGamePhaseStep() < Event->GetActivationGamePhaseStep() || GameState->GetSafeZonePhase() < Event->GetActivationSafeZoneIndex())
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
        if ((!Category || EventInstance.Category == Category) && (!Event || EventInstance.Event == Event))
            ActorCount += EventInstance.ActorCount;
    }

    return ActorCount;
}

AActor* UFortAthenaLivingWorldManager::GetRandomPointProvider(const FGameplayTagQuery& ProviderFiltersTagQuery, const FFortAthenaLivingWorldPointProviderFilterRules& PointFilter)
{
    TArray<AActor*> PointProviders = GetPointProviders();
    TArray<AActor*> ValidPointProviders;

    PointProviders.Append(GetRuntimePointProviderList());

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

bool UFortAthenaLivingWorldManager::RequestSpawnToAISpawnerSystem(FFortAthenaLivingWorldCategory* Category, FFortAthenaLivingWorldEvent* Event, UFortAthenaLivingWorldEventData* EventData)
{
    auto AISystem = Cast<UAthenaAISystem>(GetWorld()->GetAISystem());
    auto AISpawner = AISystem ? AISystem->GetAISpawner() : nullptr;
    const FFortAthenaLivingWorldPointProviderFilterRules PointFilter{ Event->GetbOnlyInSafeZone(), Event->GetbCheckForActorDensity() };
    auto PointProvider = AISpawner ? GetRandomPointProvider(EventData->GetProviderFiltersTagQuery(), PointFilter) : nullptr;

    if (!PointProvider)
        return false;

    auto PointProviderInterface = (IFortAthenaLivingWorldPointProviderInterface*)PointProvider->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass());
    FLivingWorldEventInstance EventInstance;

    EventInstance.Category = Category;
    EventInstance.Event = Event;
    EventInstance.PointProvider = PointProvider;

    for (auto& ActorDescription : EventData->GetActorDescriptions())
    {
        auto SpawnerDataClass = GetSpawnerDataClass(ActorDescription);
        auto ComponentList = SpawnerDataClass ? UFortAthenaAISpawnerData::CreateComponentListFromClass(SpawnerDataClass, this) : nullptr;
        FVector Location;
        FRotator Rotation;

        if (!ComponentList || !PointProviderInterface->GetValidLocation(PointFilter, Location, Rotation))
            continue;

        PointProviderInterface->OnEventRequested();

        const int32 RequestID = AISpawner->RequestSpawn(ComponentList, FTransform(Rotation, Location, FVector(1.0f, 1.0f, 1.0f)));

        if (RequestID == INDEX_NONE)
        {
            PointProviderInterface->OnSpawnedActorFailed();
            continue;
        }

        EventInstance.PendingRequestIDs.Add(RequestID);
        EventInstance.ActorCount++;
    }

    if (!EventInstance.ActorCount)
        return false;

    FindOrCreateEventRuntimeData(Event).SpawnedCount += EventInstance.ActorCount;
    GetEventInstances().Add(EventInstance);

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
    for (int32 Index = EventRuntimeData.Num() - 1; Index >= 0; Index--)
    {
        if (EventRuntimeData[Index].LivingWorldManager.Get() != this)
            EventRuntimeData.RemoveAt(Index);
        else if (EventRuntimeData[Index].Event == Event)
            return EventRuntimeData[Index];
    }

    FLivingWorldEventRuntimeData RuntimeData;
    RuntimeData.LivingWorldManager = this;
    RuntimeData.Event = Event;

    return EventRuntimeData[EventRuntimeData.Add(RuntimeData)];
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

TArray<AActor*>& UFortAthenaLivingWorldManager::GetPointProviders()
{
    static auto PointProviders = GetClass()->FindPropertyByName(FName(TEXT("PointProviders")));

    return *PointProviders->ContainerPtrToValuePtr<TArray<AActor*>>(this);
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

TArray<UFortAthenaLivingWorldManager::FLivingWorldEventInstance>& UFortAthenaLivingWorldManager::GetEventInstances()
{
    return *(TArray<FLivingWorldEventInstance>*)((uint8*)this + GetEventInstancesOffset());
}

float& UFortAthenaLivingWorldManager::GetNextEventGenerationTime()
{
    static auto NextEventGenerationTime = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::NextEventGenerationTime"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnCurrentPlaylistLoaded"))
        .Find("mov rcx, *; call 0x*; mov rbx, [rsp+0x*]")
        .Find("call 0x*")
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
        .Disp();

    return *(FGameplayTagContainer*)((uint8*)this + PlaylistContextTags);
}
