#include "pch.h"
#include "Conversation/FortConversationTaskNode_SupplyDrop.h"
#include "Conversation/FortConversationContextLibrary.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortGameStateAthena.h"
#include "Player/FortControllerEffect.h"
#include "Player/FortPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "HAL/IConsoleManager.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

struct FSupplyDropUsedStock
{
    TWeakObjectPtr<UConversationParticipantComponent> NPCComponent;
    const UFortConversationTaskNode_SupplyDrop* Service;
    int32 Count;
};

static TArray<FSupplyDropUsedStock> UsedStock;
static TArray<FPendingSupplyDropInfo> PendingSupplyDrops;

UClass* UFortConversationTaskNode_SupplyDrop::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationTaskNode_SupplyDrop"));

    return Class;
}

FConversationTaskResult UFortConversationTaskNode_SupplyDrop::ExecuteTaskNode_Implementation(const FConversationContext& Context) const
{
    auto PlayerController = UFortConversationContextLibrary::GetParticipantPlayerController(Context);
    auto NPCComponent = UFortConversationContextLibrary::GetParticipantComponentNPC(Context);
    auto NPCActor = UFortConversationContextLibrary::GetParticipantActorNPC(Context);
    auto World = NPCActor ? NPCActor->GetWorld() : nullptr;

    if (!PlayerController || !NPCComponent || !World || !GetSupplyDropInfo())
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    FSupplyDropUsedStock* Stock = nullptr;

    for (int32 Index = UsedStock.Num() - 1; Index >= 0; Index--)
    {
        if (!UsedStock[Index].NPCComponent.Get())
        {
            UsedStock.RemoveAt(Index);
            continue;
        }

        if (UsedStock[Index].NPCComponent.Get() == NPCComponent && UsedStock[Index].Service == this)
            Stock = &UsedStock[Index];
    }

    if (GetbLimitedStock() && Stock && Stock->Count >= (int32)GetScalableFloat(TEXT("LimitedStockCount")).GetValueAtLevel(0.0f))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const TOptional<int32> Price = FindServicePrice(FName(TEXT("supplydrop")), 0);

    if (!Price.IsSet() || !PayPrice(Context, Price))
        return FConversationTaskResult::ReturnToCurrentClientChoice();

    const int32 SupplyDropCount = (int32)GetScalableFloat(TEXT("SupplyDropCount")).GetValueAtLevel(0.0f);
    const int32 GuaranteedCloseRangeSupplyDropCount = (int32)GetScalableFloat(TEXT("GuaranteedCloseRangeSupplyDropCount")).GetValueAtLevel(0.0f);
    const float ProcessPendingSupplyDropInterval = GetScalableFloat(TEXT("ProcessPendingSupplyDropInterval")).GetValueAtLevel(0.0f);
    float ProcessTime = World->GetTimeSeconds();

    for (int32 Index = 0; Index < SupplyDropCount; Index++)
    {
        const bool bCloseRange = Index < GuaranteedCloseRangeSupplyDropCount;

        FPendingSupplyDropInfo Info;
        Info.World = World;
        Info.SupplyDropNode = (UObject*)this;
        Info.SupplyDropLocationPolicy = bCloseRange ? ESupplyDropSpawnLocationPolicy::RadiusAroundLocation : GetSupplyDropLocationPolicy();
        Info.Radius = GetScalableFloat(bCloseRange ? TEXT("GuaranteedCloseRangeRadiusAroundNPC") : TEXT("RadiusAroundNPC")).GetValueAtLevel(0.0f);
        Info.SpawnLocation = NPCActor->GetActorLocation();
        Info.ProcessTime = ProcessTime;

        PendingSupplyDrops.Add(Info);
        ProcessTime += ProcessPendingSupplyDropInterval;
    }

    if (Stock)
        Stock->Count++;
    else
        UsedStock.Add(FSupplyDropUsedStock{ NPCComponent, this, 1 });

    UFortControllerEffect_TriggerCustomFeedMessage::TriggerCustomFeedMessage(PlayerController, GetCustomFeedMessageTag(), FVector(ForceInit));

    return FConversationTaskResult::AdvanceConversation();
}

void UFortConversationTaskNode_SupplyDrop::ProcessSupplyDrop(const FPendingSupplyDropInfo& Info) const
{
    static auto DisableSupplyDropsSpawningOverWater = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("Athena.DisableSupplyDropsSpawningOverWater"));

    auto World = Info.World.Get();
    auto SupplyDropInfo = GetSupplyDropInfo();
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;

    if (!SupplyDropInfo || !MapInfo || Info.SupplyDropLocationPolicy != ESupplyDropSpawnLocationPolicy::RadiusAroundLocation)
        return;

    static auto SupplyDropClass = SupplyDropInfo->GetClass()->FindPropertyByName(FName(TEXT("SupplyDropClass")));
    static auto MinTraceHeight = SupplyDropInfo->GetClass()->FindPropertyByName(FName(TEXT("MinTraceHeight")));
    static auto MaxTraceHeight = SupplyDropInfo->GetClass()->FindPropertyByName(FName(TEXT("MaxTraceHeight")));

    const float TraceStartZ = MinTraceHeight->ContainerPtrToValuePtr<FScalableFloat>(SupplyDropInfo)->GetValueAtLevel(0.0f);
    const float TraceEndZ = MaxTraceHeight->ContainerPtrToValuePtr<FScalableFloat>(SupplyDropInfo)->GetValueAtLevel(0.0f);
    const FVector SpawnLocation = MapInfo->PickSupplyDropLocation(Info.SpawnLocation, Info.Radius, DisableSupplyDropsSpawningOverWater && DisableSupplyDropsSpawningOverWater->GetValueOnAnyThread() == 1, TraceStartZ, TraceEndZ);
    const FRotator SpawnRotation{ 0.0f, FMath::FRand() * 360.0f, 0.0f };

    MapInfo->SpawnSupplyDropAtLocation(SpawnLocation, SpawnRotation, *SupplyDropClass->ContainerPtrToValuePtr<UClass*>(SupplyDropInfo), TraceStartZ, TraceEndZ);
}

void UFortConversationTaskNode_SupplyDrop::TickPendingSupplyDrops()
{
    TArray<FPendingSupplyDropInfo> DueSupplyDrops;

    for (int32 Index = PendingSupplyDrops.Num() - 1; Index >= 0; Index--)
    {
        auto World = PendingSupplyDrops[Index].World.Get();
        auto SupplyDropNode = PendingSupplyDrops[Index].SupplyDropNode.Get();

        if (World && SupplyDropNode && World->GetTimeSeconds() < PendingSupplyDrops[Index].ProcessTime)
            continue;

        if (World && SupplyDropNode)
            DueSupplyDrops.Add(PendingSupplyDrops[Index]);

        PendingSupplyDrops.RemoveAt(Index);
    }

    for (int32 Index = DueSupplyDrops.Num() - 1; Index >= 0; Index--)
    {
        if (auto SupplyDropNode = (const UFortConversationTaskNode_SupplyDrop*)DueSupplyDrops[Index].SupplyDropNode.Get())
            SupplyDropNode->ProcessSupplyDrop(DueSupplyDrops[Index]);
    }
}

UObject* UFortConversationTaskNode_SupplyDrop::GetSupplyDropInfo() const
{
    static auto SupplyDropInfo = StaticClass()->FindPropertyByName(FName(TEXT("SupplyDropInfo")));

    return SupplyDropInfo->ContainerPtrToValuePtr<TSoftObjectPtr<UObject>>((void*)this)->LoadSynchronous();
}

const FScalableFloat& UFortConversationTaskNode_SupplyDrop::GetScalableFloat(const TCHAR* PropertyName) const
{
    return *StaticClass()->FindPropertyByName(FName(PropertyName))->ContainerPtrToValuePtr<FScalableFloat>(this);
}

ESupplyDropSpawnLocationPolicy UFortConversationTaskNode_SupplyDrop::GetSupplyDropLocationPolicy() const
{
    static auto SupplyDropLocationPolicy = StaticClass()->FindPropertyByName(FName(TEXT("SupplyDropLocationPolicy")));

    return *SupplyDropLocationPolicy->ContainerPtrToValuePtr<ESupplyDropSpawnLocationPolicy>(this);
}

bool UFortConversationTaskNode_SupplyDrop::GetbLimitedStock() const
{
    static auto bLimitedStock = StaticClass()->FindPropertyByName(FName(TEXT("bLimitedStock")));

    return *bLimitedStock->ContainerPtrToValuePtr<bool>(this);
}

const FGameplayTag& UFortConversationTaskNode_SupplyDrop::GetCustomFeedMessageTag() const
{
    static auto CustomFeedMessageTag = StaticClass()->FindPropertyByName(FName(TEXT("CustomFeedMessageTag")));

    return *CustomFeedMessageTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}
