#pragma once

#include "Conversation/FortConversationTaskNode_Service.h"
#include "GameplayTagContainer.h"
#include "Math/Vector.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UWorld;
struct FScalableFloat;

enum class ESupplyDropSpawnLocationPolicy : uint8
{
    RadiusAroundLocation,
    SafeZone
};

struct FPendingSupplyDropInfo
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<UObject> SupplyDropNode;
    ESupplyDropSpawnLocationPolicy SupplyDropLocationPolicy;
    float Radius;
    FVector SpawnLocation;
    float ProcessTime;
};

class UFortConversationTaskNode_SupplyDrop : public UFortConversationTaskNode_Service
{
public:
    static UClass* StaticClass();

    FConversationTaskResult ExecuteTaskNode_Implementation(const FConversationContext& Context) const;

    void GatherStaticExtraData_Implementation(const FConversationContext& Context, TArray<FConversationNodeParameterPair>& InOutExtraData) const;

    void ProcessSupplyDrop(const FPendingSupplyDropInfo& Info) const;

    static void TickPendingSupplyDrops();

    UObject* GetSupplyDropInfo() const;

    const FScalableFloat& GetScalableFloat(const TCHAR* PropertyName) const;

    ESupplyDropSpawnLocationPolicy GetSupplyDropLocationPolicy() const;

    bool GetbLimitedStock() const;

    const FGameplayTag& GetCustomFeedMessageTag() const;
};
