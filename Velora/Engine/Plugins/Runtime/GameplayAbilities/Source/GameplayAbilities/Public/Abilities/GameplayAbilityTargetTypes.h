#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/SharedPointer.h"
#include "Delegates/Delegate.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "GameplayPrediction.h"

struct FGameplayAbilityTargetData;

struct FGameplayAbilityTargetDataHandle
{
    TArray<TSharedPtr<FGameplayAbilityTargetData>, TInlineAllocator<1>> Data;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FAbilityTargetDataSetDelegate, const FGameplayAbilityTargetDataHandle&, FGameplayTag);

namespace EAbilityGenericReplicatedEvent
{
    enum Type
    {
        GenericConfirm = 0,
        GenericCancel,
        InputPressed,
        InputReleased,
        GenericSignalFromClient,
        GenericSignalFromServer,
        GameCustom1,
        GameCustom2,
        GameCustom3,
        GameCustom4,
        GameCustom5,
        GameCustom6,
        MAX
    };
}

struct FAbilityReplicatedData
{
    bool bTriggered;

    FVector_NetQuantize100 VectorPayload;

    FSimpleMulticastDelegate Delegate;
};

struct FAbilityReplicatedDataCache
{
    FGameplayAbilityTargetDataHandle TargetData;

    FGameplayTag ApplicationTag;

    bool bTargetConfirmed;

    bool bTargetCancelled;

    FAbilityTargetDataSetDelegate TargetSetDelegate;

    FSimpleMulticastDelegate TargetCancelledDelegate;

    FAbilityReplicatedData GenericEvents[EAbilityGenericReplicatedEvent::MAX];

    FPredictionKey PredictionKey;

    void Reset();
};
